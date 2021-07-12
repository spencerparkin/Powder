#include "EditorApp.h"
#include "EditorFrame.h"
#include <wx/filename.h>

wxIMPLEMENT_APP(EditorApp);

EditorApp::EditorApp()
{
	this->frame = nullptr;
	this->config = nullptr;
	this->runThread = nullptr;
}

/*virtual*/ EditorApp::~EditorApp()
{
	delete this->config;
}

/*virtual*/ bool EditorApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	this->config = new wxConfig("PowderEditor");
	this->projectDirectory = this->config->Read("projectDirectory");

	wxFileName fileName(this->projectDirectory);
	if (!fileName.Exists())
		this->projectDirectory = "";

	this->frame = new EditorFrame(nullptr, wxDefaultPosition, wxSize(1000, 800));
	this->frame->RestoreWindowAdjustments();
	this->frame->Show();

	return true;
}

/*virtual*/ int EditorApp::OnExit()
{
	if (this->runThread)
	{
		// TODO: Signal exit, then wait with time-out.  If timed-out, kill?
	}

	return 0;
}

void EditorApp::SetProjectDirectory(const wxString& projectDirectory)
{
	this->projectDirectory = projectDirectory;
	this->config->Write("projectDirectory", this->projectDirectory);
}

const wxString& EditorApp::GetProjectDirectory()
{
	return this->projectDirectory;
}