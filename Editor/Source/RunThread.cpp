#include "RunThread.h"
#include "GarbageCollector.h"
#include <wx/stopwatch.h>

wxDEFINE_EVENT(EVT_RUNTHREAD_ENTERING, wxThreadEvent);
wxDEFINE_EVENT(EVT_RUNTHREAD_EXITING, wxThreadEvent);
wxDEFINE_EVENT(EVT_RUNTHREAD_EXCEPTION, RunThreadExceptionEvent);
wxDEFINE_EVENT(EVT_RUNTHREAD_OUTPUT, RunThreadOutputEvent);
wxDEFINE_EVENT(EVT_RUNTHREAD_INPUT, RunThreadInputEvent);

RunThreadExceptionEvent::RunThreadExceptionEvent(Powder::Exception* exception) : wxThreadEvent(EVT_RUNTHREAD_EXCEPTION)
{
	this->errorMsg = (const char*)exception->GetErrorMessage().c_str();
}

/*virtual*/ RunThreadExceptionEvent::~RunThreadExceptionEvent()
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

RunThread::RunThread(const wxString& sourceFilePath, wxEvtHandler* eventHandler) : wxThread(wxTHREAD_JOINABLE), inputSemaphore(0, 1)
{
	this->eventHandler = eventHandler;
	this->vm = nullptr;
	this->sourceFilePath = sourceFilePath;
	this->exitNow = false;
	this->executionTimeMilliseconds = 0L;
}

/*virtual*/ RunThread::~RunThread()
{
}

/*virtual*/ wxThread::ExitCode RunThread::Entry()
{
	using namespace Powder;

	if (this->eventHandler)
		::wxQueueEvent(this->eventHandler, new wxThreadEvent(EVT_RUNTHREAD_ENTERING));

	wxStopWatch stopWatch;

	this->vm = new VirtualMachine();
	this->vm->SetDebuggerTrap(this);
	this->vm->SetIODevice(this);

	try
	{
		vm->ExecuteSourceCodeFile((const char*)this->sourceFilePath.c_str());
	}
	catch (Exception* exc)
	{
		if (this->eventHandler)
			::wxQueueEvent(this->eventHandler, new RunThreadExceptionEvent(exc));
		delete exc;
	}

	delete this->vm;
	GarbageCollector::GC()->FullPurge();

	this->executionTimeMilliseconds = stopWatch.Time();

	if (this->eventHandler)
		::wxQueueEvent(this->eventHandler, new wxThreadEvent(EVT_RUNTHREAD_EXITING));

	return 0;
}

void RunThread::SignalExit(bool appGoingDown /*= false*/)
{
	if (appGoingDown)
		this->eventHandler = nullptr;
	this->exitNow = true;
	this->inputSemaphore.Post();	// This is fine, because the semephore is re-created with each thread.
}

/*virtual*/ bool RunThread::TrapExecution(const Powder::Executable* executable, Powder::Executor* executor)
{
	// TODO: Block on a semaphore here if we're stopped at a break-point.
	//       When we are blocked, it should be save for the main thread to
	//       examine the variables inside the VM we own.

	return this->exitNow;
}

/*virtual*/ void RunThread::InputString(std::string& str)
{
	wxString inputText;
	::wxQueueEvent(this->eventHandler, new RunThreadInputEvent(&inputText));
	this->inputSemaphore.Wait();
	str = (const char*)inputText.c_str();
}

/*virtual*/ void RunThread::OutputString(const std::string& str)
{
	::wxQueueEvent(this->eventHandler, new RunThreadOutputEvent((const char*)str.c_str()));
}