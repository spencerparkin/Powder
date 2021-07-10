#include "EditorFrame.h"
#include "DirectoryTreeControl.h"
#include "SourceFileNotebookControl.h"
#include "EditorApp.h"
#include <wx/menu.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <wx/dirdlg.h>
#include <wx/aboutdlg.h>

EditorFrame::EditorFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Powder Editor", pos, size)
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SaveAll, "Save All", "Save all currently open files."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_CloseAll, "Close All", "Close all currently open files."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_OpenDirectory, "Open Directory", "Open a given directory to work with and edit all powder files in that directory."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_CloseDirectory, "Close Directory", "Close the currently open directory, if any."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Go skiing."));

	wxMenu* debugMenu = new wxMenu();
	debugMenu->Append(new wxMenuItem(debugMenu, ID_RunWithDebugger, "Run with Debugger", "Run the currently shown script with the debugger attached."));
	debugMenu->Append(new wxMenuItem(debugMenu, ID_RunWithoutDebugger, "Run without Debugger", "Run the currently shown script without a debugger attached."));
	debugMenu->AppendSeparator();
	debugMenu->Append(new wxMenuItem(debugMenu, ID_AttachToVM, "Attach to VM", "Attach to a Powder VM.  This could be the one embedded in this editor, or one elsewhere."));
	debugMenu->Append(new wxMenuItem(debugMenu, ID_DetachFromVM, "Detach from VM", "Detach from the currently attached Powder VM, if any."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(debugMenu, "Debug");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->SetStatusBar(new wxStatusBar(this));

	this->Bind(wxEVT_MENU, &EditorFrame::OnSaveAll, this, ID_SaveAll);
	this->Bind(wxEVT_MENU, &EditorFrame::OnCloseAll, this, ID_CloseAll);
	this->Bind(wxEVT_MENU, &EditorFrame::OnOpenDirectory, this, ID_OpenDirectory);
	this->Bind(wxEVT_MENU, &EditorFrame::OnCloseDirectory, this, ID_CloseDirectory);
	this->Bind(wxEVT_MENU, &EditorFrame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &EditorFrame::OnRunWithDebugger, this, ID_RunWithDebugger);
	this->Bind(wxEVT_MENU, &EditorFrame::OnRunWithoutDebugger, this, ID_RunWithoutDebugger);
	this->Bind(wxEVT_MENU, &EditorFrame::OnAttachToVM, this, ID_AttachToVM);
	this->Bind(wxEVT_MENU, &EditorFrame::OnDetachFromVM, this, ID_DetachFromVM);
	this->Bind(wxEVT_MENU, &EditorFrame::OnAbout, this, ID_About);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_SaveAll);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseAll);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_OpenDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_RunWithDebugger);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_RunWithoutDebugger);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_AttachToVM);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_DetachFromVM);
	this->Bind(wxEVT_CLOSE_WINDOW, &EditorFrame::OnClose, this);

	wxSplitterWindow* verticalSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D);

	this->directoryTreeControl = new DirectoryTreeControl(verticalSplitter);
	this->sourceFileNotebookControl = new SourceFileNotebookControl(verticalSplitter);

	verticalSplitter->SplitVertically(this->directoryTreeControl, this->sourceFileNotebookControl);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(verticalSplitter, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);
}

/*virtual*/ EditorFrame::~EditorFrame()
{
}

void EditorFrame::OnExit(wxCommandEvent& event)
{
	this->Close();
}

void EditorFrame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Powder Editor");
	aboutDialogInfo.SetVersion("1.0");
	aboutDialogInfo.SetDescription("The Powder VM and associated tools were written by Spencer T. Parkin as a purely academic exercise, and because he had nothing better to do.");

	wxAboutBox(aboutDialogInfo);
}

void EditorFrame::OnSaveAll(wxCommandEvent& event)
{
	this->sourceFileNotebookControl->SaveAllFiles();
}

void EditorFrame::OnCloseAll(wxCommandEvent& event)
{
	this->sourceFileNotebookControl->CloseAllFiles();
}

void EditorFrame::OnOpenDirectory(wxCommandEvent& event)
{
	wxDirDialog dirDialog(this, "Open which directory?", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (wxID_OK == dirDialog.ShowModal())
	{
		wxGetApp().projectDirectory = dirDialog.GetPath();
		this->directoryTreeControl->RebuildForDirectory(wxGetApp().projectDirectory);
	}
}

void EditorFrame::OnCloseDirectory(wxCommandEvent& event)
{
	if (this->sourceFileNotebookControl->CloseAllFiles())
	{
		this->directoryTreeControl->DeleteAllItems();
		wxGetApp().projectDirectory = "";
	}
}

void EditorFrame::OnRunWithDebugger(wxCommandEvent& event)
{
}

void EditorFrame::OnRunWithoutDebugger(wxCommandEvent& event)
{
}

void EditorFrame::OnAttachToVM(wxCommandEvent& event)
{
}

void EditorFrame::OnDetachFromVM(wxCommandEvent& event)
{
}

void EditorFrame::OnUpdateMenuItemUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_SaveAll:
		{
			event.Enable(this->sourceFileNotebookControl->AnyFilesModified());
			break;
		}
		case ID_CloseAll:
		{
			event.Enable(this->sourceFileNotebookControl->OpenFileCount() > 0);
			break;
		}
		case ID_OpenDirectory:
		{
			event.Enable(wxGetApp().projectDirectory.IsEmpty());
			break;
		}
		case ID_CloseDirectory:
		{
			event.Enable(!wxGetApp().projectDirectory.IsEmpty());
			break;
		}
	}
}

void EditorFrame::OnClose(wxCloseEvent& event)
{
	if (this->sourceFileNotebookControl->CloseAllFiles())
		wxFrame::OnCloseWindow(event);
}