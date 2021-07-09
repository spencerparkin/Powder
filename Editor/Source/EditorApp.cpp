#include "EditorApp.h"
#include "EditorFrame.h"

wxIMPLEMENT_APP(EditorApp);

EditorApp::EditorApp()
{
	this->frame = nullptr;
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

	// TODO: This app embeds the Powder VM and attaches to it by default, but you could attach to some
	//       other Powder VM embedded elsewhere if you want.

	return true;
}

/*virtual*/ int EditorApp::OnExit()
{
	return 0;
}