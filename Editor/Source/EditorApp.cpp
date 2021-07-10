#include "EditorApp.h"
#include "EditorFrame.h"

wxIMPLEMENT_APP(EditorApp);

EditorApp::EditorApp()
{
	this->frame = nullptr;
	this->vmAttachment = nullptr;
	this->vmRun = nullptr;
}

/*virtual*/ EditorApp::~EditorApp()
{
}

/*virtual*/ bool EditorApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	this->frame = new EditorFrame(nullptr, wxDefaultPosition, wxSize(1000, 800));
	this->frame->Show();

	return true;
}

/*virtual*/ int EditorApp::OnExit()
{
	return 0;
}