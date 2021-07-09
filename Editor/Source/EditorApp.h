#pragma once

#include <wx/setup.h>
#include <wx/app.h>

class EditorApp : public wxApp
{
public:
	EditorApp();
	virtual ~EditorApp();

	virtual bool OnInit() override;
	virtual int OnExit() override;
};

wxDECLARE_APP(EditorApp);