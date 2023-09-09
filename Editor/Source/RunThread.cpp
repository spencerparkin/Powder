#include "RunThread.h"
#include "EditorApp.h"
#include "GarbageCollector.h"
#include "Executable.h"
#include "Executor.h"
#include <wx/stopwatch.h>

wxDEFINE_EVENT(EVT_RUNTHREAD_ENTERING, wxThreadEvent);
wxDEFINE_EVENT(EVT_RUNTHREAD_EXITING, wxThreadEvent);
wxDEFINE_EVENT(EVT_RUNTHREAD_ERROR, RunThreadErrorEvent);
wxDEFINE_EVENT(EVT_RUNTHREAD_OUTPUT, RunThreadOutputEvent);
wxDEFINE_EVENT(EVT_RUNTHREAD_INPUT, RunThreadInputEvent);
wxDEFINE_EVENT(EVT_RUNTHREAD_SUSPENDED, RunThreadSuspendedEvent);

RunThreadErrorEvent::RunThreadErrorEvent(Powder::Error& error) : wxThreadEvent(EVT_RUNTHREAD_ERROR)
{
	this->errorMsg = wxString((const char*)std::string(error).c_str());
}

/*virtual*/ RunThreadErrorEvent::~RunThreadErrorEvent()
{
}

RunThreadOutputEvent::RunThreadOutputEvent(const wxString& outputText) : wxThreadEvent(EVT_RUNTHREAD_OUTPUT)
{
	this->outputText = outputText;
}

/*virtual*/ RunThreadOutputEvent::~RunThreadOutputEvent()
{
}

RunThreadInputEvent::RunThreadInputEvent(wxString* inputText) : wxThreadEvent(EVT_RUNTHREAD_INPUT)
{
	this->inputText = inputText;
}

/*virtual*/ RunThreadInputEvent::~RunThreadInputEvent()
{
}

RunThreadSuspendedEvent::RunThreadSuspendedEvent(const wxString& sourceFile, int lineNumber, int columnNumber, uint64_t programBufferLocation, const ParseParty::JsonObject* instructionMapValue) : wxThreadEvent(EVT_RUNTHREAD_SUSPENDED)
{
	this->sourceFile = sourceFile;
	this->lineNumber = lineNumber;
	this->columnNumber = columnNumber;
	this->programBufferLocation = programBufferLocation;
	this->instructionMapValue = instructionMapValue;
}

/*virtual*/ RunThreadSuspendedEvent::~RunThreadSuspendedEvent()
{
}

RunThread::RunThread(const wxString& sourceFilePath, wxEvtHandler* eventHandler, bool debuggingEnabled) : wxThread(wxTHREAD_JOINABLE), suspensionSemaphore(0, 1)
{
	this->debuggingEnabled = debuggingEnabled;
	this->suspensionState = NOT_SUSPENDED;
	this->resumeState = RESUME_HAPPY;
	this->suspendNow = debuggingEnabled;
	this->exitNow = false;
	this->eventHandler = eventHandler;
	this->vm = nullptr;
	this->sourceFilePath = sourceFilePath;
	this->executionTimeMilliseconds = 0L;
	this->callDepth = 0;
	this->targetCallDepth = -1;
	this->avoidLineNumber = -1;
	this->keepLineNumber = -1;
	this->prevLineNumber = -1;
}

/*virtual*/ RunThread::~RunThread()
{
}

/*virtual*/ wxThread::ExitCode RunThread::Entry()
{
	using namespace Powder;

	::wxQueueEvent(this->eventHandler, new wxThreadEvent(EVT_RUNTHREAD_ENTERING));
	wxStopWatch stopWatch;
	
	GC::GarbageCollector* gc = new GC::GarbageCollector();
	GC::GarbageCollector::Set(gc);

	this->vm = new VirtualMachine();
	this->vm->SetDebuggerTrap(this);
	this->vm->SetIODevice(this);

	Error error;
	if (!this->vm->ExecuteSourceCodeFile((const char*)this->sourceFilePath.c_str(), error))
		::wxQueueEvent(this->eventHandler, new RunThreadErrorEvent(error));

	delete this->vm;
	delete gc;
	GC::GarbageCollector::Set(nullptr);

	this->executionTimeMilliseconds = stopWatch.Time();
	::wxQueueEvent(this->eventHandler, new wxThreadEvent(EVT_RUNTHREAD_EXITING));

	return 0;
}

/*virtual*/ bool RunThread::TrapExecution(const Powder::Executable* executable, Powder::Executor* executor)
{
	if (this->exitNow)
		return true;

	if (this->debuggingEnabled)
	{
		int lineNumber = -1;
		int columnNumber = -1;

		const ParseParty::JsonObject* instructionMapValue = dynamic_cast<const ParseParty::JsonObject*>(executable->debugInfoDoc->GetValue("instruction_map"));
		if (instructionMapValue)
		{
			char key[32];
			::sprintf_s(key, sizeof(key), "%lu", (unsigned long)executor->GetProgramBufferLocation());
			const ParseParty::JsonObject* instructionEntryValue = dynamic_cast<const ParseParty::JsonObject*>(instructionMapValue->GetValue(key));
			if (instructionEntryValue)
			{
				const ParseParty::JsonString* debuggerHelpValue = dynamic_cast<const ParseParty::JsonString*>(instructionEntryValue->GetValue("debugger_help"));
				if (debuggerHelpValue)
				{
					wxString debuggerHelp = debuggerHelpValue->GetValue();
					if (debuggerHelp.EndsWith("_call"))
						this->callDepth++;
					else if (debuggerHelp.EndsWith("_return"))
						this->callDepth--;
				}

				const ParseParty::JsonInt* lineValue = dynamic_cast<const ParseParty::JsonInt*>(instructionEntryValue->GetValue("line"));
				if (lineValue)
					lineNumber = lineValue->GetValue();

				const ParseParty::JsonInt* colValue = dynamic_cast<const ParseParty::JsonInt*>(instructionEntryValue->GetValue("col"));
				if (colValue)
					columnNumber = colValue->GetValue();
			}
		}

		if (this->prevLineNumber != lineNumber)
		{
			this->prevLineNumber = lineNumber;

			const ParseParty::JsonString* sourceFileValue = dynamic_cast<const ParseParty::JsonString*>(executable->debugInfoDoc->GetValue("source_file"));
			if (sourceFileValue)
			{
				wxFileName sourceFile = wxString(sourceFileValue->GetValue());
				wxCriticalSectionLocker locker(wxGetApp().breakpointListCS);
				EditorApp::Breakpoint* breakpoint = wxGetApp().FindBreakpoint(sourceFile, lineNumber);
				if (breakpoint)
					this->suspendNow = true;
			}
		}

		if (this->resumeState == RESUME_STEP_OUT && this->callDepth == this->targetCallDepth)
			this->suspendNow = true;

		if (this->suspendNow)
		{
			this->suspensionState = SUSPENDED_FOR_DEBUG;

			wxString sourceFile;
			const ParseParty::JsonString* sourceFileValue = dynamic_cast<const ParseParty::JsonString*>(executable->debugInfoDoc->GetValue("source_file"));
			if (sourceFileValue)
				sourceFile = sourceFileValue->GetValue();

			::wxQueueEvent(this->eventHandler, new RunThreadSuspendedEvent(sourceFile, lineNumber, columnNumber, executor->GetProgramBufferLocation(), instructionMapValue));

			this->suspensionSemaphore.Wait();
			this->suspendNow = false;
			this->targetCallDepth = -1;
			this->avoidLineNumber = -1;
			this->keepLineNumber = -1;

			switch (this->resumeState)
			{
				case RESUME_STEP_OVER:
				{
					this->avoidLineNumber = lineNumber;
					this->targetCallDepth = this->callDepth;
					break;
				}
				case RESUME_STEP_INTO:
				{
					this->keepLineNumber = lineNumber;
					this->targetCallDepth = this->callDepth + 1;
					break;
				}
				case RESUME_STEP_OUT:
				{
					this->targetCallDepth = this->callDepth - 1;
					break;
				}
			}
		}

		if (this->resumeState == RESUME_STEP_INTO && (this->callDepth == this->targetCallDepth || lineNumber != this->keepLineNumber))
			this->suspendNow = true;

		if (this->resumeState == RESUME_STEP_OVER && lineNumber != this->avoidLineNumber && this->callDepth <= this->targetCallDepth)
			this->suspendNow = true;
	}

	return false;
}

/*virtual*/ void RunThread::ValueChanged(const Powder::Value* value)
{
}

/*virtual*/ void RunThread::ValueStored(const char* name, const Powder::Value* value)
{
}

/*virtual*/ void RunThread::ValueLoaded(const char* name, const Powder::Value* value)
{
}

/*virtual*/ void RunThread::InputString(std::string& str)
{
	wxString inputText;
	::wxQueueEvent(this->eventHandler, new RunThreadInputEvent(&inputText));
	this->suspensionState = SUSPENDED_FOR_INPUT;
	this->suspensionSemaphore.Wait();
	str = (const char*)inputText.c_str();
}

/*virtual*/ void RunThread::OutputString(const std::string& str)
{
	::wxQueueEvent(this->eventHandler, new RunThreadOutputEvent((const char*)str.c_str()));
}

void RunThread::MainThread_Pause(void)
{
	this->suspendNow = true;
}

void RunThread::MainThread_Resume(void)
{
	this->suspensionState = NOT_SUSPENDED;
	this->resumeState = RESUME_HAPPY;
	this->suspensionSemaphore.Post();
}

void RunThread::MainThread_ExitNow(void)
{
	this->exitNow = true;
	this->suspensionState = NOT_SUSPENDED;
	this->suspensionSemaphore.Post();
}

void RunThread::MainThread_StepOver(void)
{
	this->suspensionState = NOT_SUSPENDED;
	this->resumeState = RESUME_STEP_OVER;
	this->suspensionSemaphore.Post();
}

void RunThread::MainThread_StepInto(void)
{
	this->suspensionState = NOT_SUSPENDED;
	this->resumeState = RESUME_STEP_INTO;
	this->suspensionSemaphore.Post();
}

void RunThread::MainThread_StepOut(void)
{
	this->suspensionState = NOT_SUSPENDED;
	this->resumeState = RESUME_STEP_OUT;
	this->suspensionSemaphore.Post();
}