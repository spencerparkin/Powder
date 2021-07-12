#include "RunThread.h"
#include "VirtualMachine.h"
#include "Exceptions.hpp"

wxDEFINE_EVENT(wxEVT_RUNTHREAD_EXITING, wxThreadEvent);

RunThread::RunThread(const wxString& sourceFilePath, wxEvtHandler* eventHandler) : wxThread(wxTHREAD_JOINABLE)
{
	this->eventHandler = eventHandler;
	this->vm = nullptr;
	this->sourceFilePath = sourceFilePath;
	this->exitNow = false;
}

/*virtual*/ RunThread::~RunThread()
{
}

/*virtual*/ wxThread::ExitCode RunThread::Entry()
{
	using namespace Powder;

	this->vm = new VirtualMachine();
	this->vm->SetDebuggerTrap(this);

	try
	{
		vm->ExecuteSourceCodeFile((const char*)this->sourceFilePath.c_str());
	}
	catch (Exception* exc)
	{
		delete exc;
	}

	delete this->vm;

	wxQueueEvent(this->eventHandler, new wxThreadEvent(wxEVT_RUNTHREAD_EXITING));
	return 0;
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
	// TODO: Signal main thread here, then block on a semaphore until we get a string resource.
}

/*virtual*/ void RunThread::OutputString(const std::string& str)
{
	// TODO: Use thread-safe way of communicating string to main thread.
}