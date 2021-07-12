#pragma once

#include <wx/thread.h>
#include <wx/string.h>
#include <wx/event.h>
#include "VirtualMachine.h"

wxDECLARE_EVENT(wxEVT_RUNTHREAD_EXITING, wxThreadEvent);

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
};