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

wxDECLARE_EVENT(EVT_RUNTHREAD_ENTERING, wxThreadEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_EXITING, wxThreadEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_EXCEPTION, RunThreadExceptionEvent);
wxDECLARE_EVENT(EVT_RUNTHREAD_OUTPUT, RunThreadOutputEvent);

class RunThread : public wxThread, Powder::VirtualMachine::DebuggerTrap, Powder::VirtualMachine::IODevice
{
public:
	RunThread(const wxString& sourceFilePath, wxEvtHandler* eventHandler);
	virtual ~RunThread();

	virtual ExitCode Entry() override;
	virtual bool TrapExecution(const Powder::Executable* executable, Powder::Executor* executor) override;
	virtual void InputString(std::string& str) override;
	virtual void OutputString(const std::string& str) override;

	wxEvtHandler* eventHandler;
	wxString sourceFilePath;
	bool exitNow;
	Powder::VirtualMachine* vm;
	long executionTimeMilliseconds;
};