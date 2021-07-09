#pragma once

#include "VirtualMachine.h"
#include "Compiler.h"
#include "Debugger.h"
#include <wx/setup.h>
#include <wx/app.h>

class EditorFrame;

class EditorApp : public wxApp
{
public:
	EditorApp();
	virtual ~EditorApp();

	virtual bool OnInit() override;
	virtual int OnExit() override;

	EditorFrame* frame;

	wxString projectDirectory;
};

wxDECLARE_APP(EditorApp);