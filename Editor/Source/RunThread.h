#pragma once

#include <wx/thread.h>
#include <wx/string.h>
#include <wx/event.h>
#include "VirtualMachine.h"
#include "Exceptions.hpp"

class RunThreadExceptionEvent : public wxThreadEvent
{
public:
	RunThreadExceptionEvent(Powder::Exception* exception);
	virtual ~RunThreadExceptionEvent();
	
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

wxDECLARE_EVENT(EVT_RUNTHREAD_ENTERING, wxThreadEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_EXITING, wxThreadEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_EXCEPTION, RunThreadExceptionEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_OUTPUT, RunThreadOutputEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_INPUT, RunThreadInputEvent);

class RunThread : public wxThread, Powder::VirtualMachine::DebuggerTrap, Powder::VirtualMachine::IODevice
{
public:
	RunThread(const wxString& sourceFilePath, wxEvtHandler* eventHandler);
	virtual ~RunThread();

	virtual ExitCode Entry() override;
	virtual bool TrapExecution(const Powder::Executable* executable, Powder::Executor* executor) override;
	virtual void InputString(std::string& str) override;
	virtual void OutputString(const std::string& str) override;

	void SignalExit(bool appGoingDown = false);

	wxEvtHandler* eventHandler;
	wxString sourceFilePath;
	bool exitNow;
	Powder::VirtualMachine* vm;
	long executionTimeMilliseconds;
	wxSemaphore inputSemaphore;
};