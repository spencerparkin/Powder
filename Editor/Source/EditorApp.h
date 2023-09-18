#pragma once

#include "VirtualMachine.h"
#include "Compiler.h"
#include "Lexer.h"
#include <wx/setup.h>
#include <wx/app.h>
#include <wx/config.h>
#include <wx/thread.h>
#include <wx/filename.h>
#include <list>

class EditorFrame;
class RunThread;

class EditorApp : public wxApp
{
public:
	EditorApp();
	virtual ~EditorApp();

	virtual bool OnInit() override;
	virtual int OnExit() override;

	void SetProjectDirectory(const wxString& projectDirectory);
	const wxString& GetProjectDirectory();
	EditorFrame* GetFrame() { return this->frame; }
	wxConfig* GetConfig() { return this->config; }
	void SetRunThread(RunThread* runThread) { this->runThread = runThread; }
	RunThread* GetRunThread() { return this->runThread; }

	struct Breakpoint
	{
		wxFileName sourceFile;
		int lineNumber;
	};

	std::list<Breakpoint> breakpointList;
	wxCriticalSection breakpointListCS;

	Breakpoint* FindBreakpoint(const wxFileName& sourceFile, int lineNumber, std::list<Breakpoint>::iterator* foundIter = nullptr);
	void ToggleBreakpoint(const wxFileName& sourceFile, int lineNumber, bool notify = true);

	ParseParty::Lexer lexer;

private:

	EditorFrame* frame;
	wxString projectDirectory;
	wxConfig* config;
	RunThread* runThread;
};

wxDECLARE_APP(EditorApp);