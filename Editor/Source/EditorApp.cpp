#include "EditorApp.h"
#include "EditorFrame.h"
#include "ArtProvider.h"
#include "PathResolver.h"
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/msgdlg.h>

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

	wxInitAllImageHandlers();

	wxArtProvider::Push(new ArtProvider(::wxGetCwd()));

	Powder::PathResolver pathResolver;
	Powder::Error error;
	std::string lexiconFilePath = pathResolver.ResolvePath("Compiler\\Lexicon.json", Powder::PathResolver::SEARCH_BASE, error);
	if (lexiconFilePath.size() == 0)
	{
		wxMessageBox("Failed to find lexicon file: " + wxString(std::string(error).c_str()), "Error!", wxICON_ERROR | wxOK, nullptr);
		return false;
	}
	else
	{
		std::string errorStr;
		if (!this->lexer.ReadFile(lexiconFilePath, errorStr))
		{
			wxMessageBox("Failed to load lexer: " + wxString(errorStr.c_str()), "Error!", wxICON_ERROR | wxOK, nullptr);
			return false;
		}
	}

	this->config = new wxConfig("PowderEditor");
	this->SetProjectDirectory(this->config->Read("projectDirectory"));

	this->frame = new EditorFrame(nullptr, wxDefaultPosition, wxSize(1000, 800));
	this->frame->RestoreWindowAdjustments();
	this->frame->Show();

	return true;
}

/*virtual*/ int EditorApp::OnExit()
{
	return 0;
}

void EditorApp::SetProjectDirectory(const wxString& projectDirectory)
{
	this->projectDirectory = projectDirectory;
	
	if (this->projectDirectory.Length())
	{
		if (wxDir::Exists(projectDirectory))
			::wxSetWorkingDirectory(this->projectDirectory);
		else
			this->projectDirectory = "";
	}

	this->config->Write("projectDirectory", this->projectDirectory);
}

const wxString& EditorApp::GetProjectDirectory()
{
	return this->projectDirectory;
}

EditorApp::Breakpoint* EditorApp::FindBreakpoint(const wxFileName& sourceFile, int lineNumber, std::list<Breakpoint>::iterator* foundIter /*= nullptr*/)
{
	for (std::list<Breakpoint>::iterator iter = this->breakpointList.begin(); iter != this->breakpointList.end(); iter++)
	{
		Breakpoint& breakpoint = *iter;
		if (breakpoint.lineNumber == lineNumber && breakpoint.sourceFile == sourceFile)
		{
			if (foundIter != nullptr)
				*foundIter = iter;
			return &breakpoint;
		}
	}
	return nullptr;
}

void EditorApp::ToggleBreakpoint(const wxFileName& sourceFile, int lineNumber, bool notify /*= true*/)
{
	std::list<Breakpoint>::iterator iter;
	if (this->FindBreakpoint(sourceFile, lineNumber, &iter))
		this->breakpointList.erase(iter);
	else
	{
		Breakpoint breakpoint;
		breakpoint.sourceFile = sourceFile;
		breakpoint.lineNumber = lineNumber;
		this->breakpointList.push_back(breakpoint);
	}

	if (notify)
		this->frame->NotifyPanels(Panel::BREAKPOINTS_CHANGED, (void*)&sourceFile);
}