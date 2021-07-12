#include "EditorFrame.h"
#include "DirectoryTreeControl.h"
#include "SourceFileNotebookControl.h"
#include "SourceFileEditControl.h"
#include "EditorApp.h"
#include <wx/menu.h>
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

	wxMenu* runMenu = new wxMenu();
	runMenu->Append(new wxMenuItem(runMenu, ID_RunWithDebugger, "Run with Debugger", "Run the currently shown script with the debugger attached."));
	runMenu->Append(new wxMenuItem(runMenu, ID_RunWithoutDebugger, "Run without Debugger", "Run the currently shown script without a debugger attached."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(runMenu, "Run");
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
	this->Bind(wxEVT_MENU, &EditorFrame::OnAbout, this, ID_About);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_SaveAll);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseAll);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_OpenDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_RunWithDebugger);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_RunWithoutDebugger);
	this->Bind(wxEVT_CLOSE_WINDOW, &EditorFrame::OnClose, this);

	this->verticalSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D);

	this->directoryTreeControl = new DirectoryTreeControl(this->verticalSplitter);
	this->sourceFileNotebookControl = new SourceFileNotebookControl(this->verticalSplitter);
	this->sourceFileNotebookControl->RestoreOpenFiles();

	this->verticalSplitter->SplitVertically(this->directoryTreeControl, this->sourceFileNotebookControl);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(verticalSplitter, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);

	verticalSplitter->SetSashPosition(200);

	this->UpdateTreeControl();
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
	aboutDialogInfo.SetDescription("The Powder VM and associated tools were written by Spencer T. Parkin as a purely academic exercise.");

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
		wxGetApp().SetProjectDirectory(dirDialog.GetPath());
		this->UpdateTreeControl();
	}
}

void EditorFrame::OnCloseDirectory(wxCommandEvent& event)
{
	if (this->sourceFileNotebookControl->CloseAllFiles())
	{
		wxGetApp().SetProjectDirectory("");
		this->UpdateTreeControl();
	}
}

void EditorFrame::UpdateTreeControl(void)
{
	this->directoryTreeControl->RebuildForDirectory(wxGetApp().GetProjectDirectory());
}

void EditorFrame::OnRunWithDebugger(wxCommandEvent& event)
{
}

void EditorFrame::OnRunWithoutDebugger(wxCommandEvent& event)
{
	SourceFileEditControl* editControl = this->sourceFileNotebookControl->GetSelectedEditControl();
	if (editControl)
	{

	}
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
			event.Enable(wxGetApp().GetProjectDirectory().IsEmpty());
			break;
		}
		case ID_CloseDirectory:
		{
			event.Enable(!wxGetApp().GetProjectDirectory().IsEmpty());
			break;
		}
		case ID_RunWithDebugger:
		case ID_RunWithoutDebugger:
		{
			SourceFileEditControl* editControl = this->sourceFileNotebookControl->GetSelectedEditControl();
			event.Enable(editControl != nullptr);
			if (editControl)
			{
				if (event.GetId() == ID_RunWithDebugger)
					event.SetText("Run " + editControl->GetFileName() + " with Debugger");
				else
					event.SetText("Run " + editControl->GetFileName() + " without Debugger");
			}
			else
			{
				if (event.GetId() == ID_RunWithDebugger)
					event.SetText("Run with Debugger");
				else
					event.SetText("Run without Debugger");
			}

			break;
		}
	}
}

void EditorFrame::SaveWindowAdjustments()
{
	wxGetApp().GetConfig()->Write("windowX", this->GetPosition().x);
	wxGetApp().GetConfig()->Write("windowY", this->GetPosition().y);
	wxGetApp().GetConfig()->Write("windowWidth", this->GetSize().x);
	wxGetApp().GetConfig()->Write("windowHeight", this->GetSize().y);
	wxGetApp().GetConfig()->Write("verticalSplitterSashPos", this->verticalSplitter->GetSashPosition());
}

void EditorFrame::RestoreWindowAdjustments()
{
	// Note that if the window title bar is off desktop, there is a trick to
	// getting it back into the desktop.  You have to access the system menu's
	// move menu item from the task bar when hold the shift key down when
	// selecting it.  Now locate your cursor, which should be a quad-arrow.
	// Then press the up-arrow key.
	wxPoint point;
	point.x = wxGetApp().GetConfig()->Read("windowX", -1);
	point.y = wxGetApp().GetConfig()->Read("windowY", -1);
	if (point.x >= 0 && point.y >= 0)
		this->SetPosition(point);

	wxSize size;
	size.x = wxGetApp().GetConfig()->Read("windowWidth", -1);
	size.y = wxGetApp().GetConfig()->Read("windowHeight", -1);
	if (size.x >= 0 && size.y >= 0)
		this->SetSize(size);

	int sashPos = wxGetApp().GetConfig()->Read("verticalSplitterSashPos", -1);
	if (sashPos >= 0)
		this->verticalSplitter->SetSashPosition(sashPos);
}

void EditorFrame::OnClose(wxCloseEvent& event)
{
	this->SaveWindowAdjustments();
	this->sourceFileNotebookControl->RememberCurrentlyOpenFiles();

	if (this->sourceFileNotebookControl->CloseAllFiles())
		wxFrame::OnCloseWindow(event);
}