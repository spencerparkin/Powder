#pragma once

#include <wx/thread.h>
#include <wx/string.h>
#include <wx/event.h>
#include <wx/filename.h>
#include "VirtualMachine.h"
#include "Error.h"
#include "Lexer.h"
#include <set>
#include <map>

class RunThreadErrorEvent : public wxThreadEvent
{
public:
	RunThreadErrorEvent(Powder::Error& error);
	virtual ~RunThreadErrorEvent();
	
	wxString errorMsg;
};

class RunThreadOutputEvent : public wxThreadEvent
{
public:
	RunThreadOutputEvent(const wxString& outputText);
	virtual ~RunThreadOutputEvent();

	wxString outputText;
};

class RunThreadInputEvent : public wxThreadEvent
{
public:
	RunThreadInputEvent(wxString* inputText);
	virtual ~RunThreadInputEvent();

	wxString* inputText;
};

class RunThreadSuspendedEvent : public wxThreadEvent
{
public:
	RunThreadSuspendedEvent(const Powder::Executable* executable, const ParseParty::Lexer::FileLocation& fileLocation, Powder::Executor* executor, const ParseParty::JsonObject* instructionMapValue);
	virtual ~RunThreadSuspendedEvent();

	const Powder::Executable* executable;
	ParseParty::Lexer::FileLocation fileLocation;
	Powder::Executor* executor;
	const ParseParty::JsonObject* instructionMapValue;
};

wxDECLARE_EVENT(EVT_RUNTHREAD_ENTERING, wxThreadEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_EXITING, wxThreadEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_ERROR, RunThreadErrorEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_OUTPUT, RunThreadOutputEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_INPUT, RunThreadInputEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_SUSPENDED, RunThreadSuspendedEvent);

class RunThread : public wxThread, Powder::VirtualMachine::DebuggerTrap, Powder::VirtualMachine::IODevice
{
public:
	RunThread(const wxString& sourceFilePath, wxEvtHandler* eventHandler, bool debuggingEnabled);
	virtual ~RunThread();

	virtual ExitCode Entry() override;
	virtual bool TrapExecution(const Powder::Executable* executable, Powder::Executor* executor) override;
	virtual void ValueChanged(const Powder::Value* value) override;
	virtual void ValueStored(const char* name, const Powder::Value* value) override;
	virtual void ValueLoaded(const char* name, const Powder::Value* value) override;
	virtual void InputString(std::string& str) override;
	virtual void OutputString(const std::string& str) override;

	void MainThread_Pause(void);
	void MainThread_Resume(void);
	void MainThread_ExitNow(void);
	void MainThread_StepOver(void);
	void MainThread_StepInto(void);
	void MainThread_StepOut(void);
	void MainThread_StepInstruction(void);

	enum SuspensionState
	{
		NOT_SUSPENDED,
		SUSPENDED_FOR_DEBUG,
		SUSPENDED_FOR_INPUT
	};

	enum ResumeState
	{
		RESUME_HAPPY,
		RESUME_STEP_OVER,
		RESUME_STEP_INTO,
		RESUME_STEP_OUT,
		RESUME_STEP_INSTRUCTION
	};

	bool debuggingEnabled;
	SuspensionState suspensionState;
	ResumeState resumeState;
	bool exitNow;
	bool suspendNow;
	wxEvtHandler* eventHandler;
	wxString sourceFilePath;
	Powder::VirtualMachine* vm;
	long executionTimeMilliseconds;
	wxSemaphore suspensionSemaphore;
	int callDepth;
	int targetCallDepth;
	int avoidLineNumber;
	int keepLineNumber;
	int prevLineNumber;
};