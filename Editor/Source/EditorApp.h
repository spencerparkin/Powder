#pragma once

#include "VirtualMachine.h"
#include "Compiler.h"
#include <wx/setup.h>
#include <wx/app.h>
#include <wx/config.h>

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

private:

	EditorFrame* frame;
	wxString projectDirectory;
	wxConfig* config;
	RunThread* runThread;
};

wxDECLARE_APP(EditorApp);