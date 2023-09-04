#include "EditorFrame.h"
#include "DirectoryTreePanel.h"
#include "DirectoryTreeControl.h"
#include "SourceFilePanel.h"
#include "SourceFileNotebookControl.h"
#include "SourceFileEditControl.h"
#include "TerminalPanel.h"
#include "TerminalControl.h"
#include "BreakpointsPanel.h"
#include "RunThread.h"
#include "EditorApp.h"
#include "ArtProvider.h"
#include "FileFinder.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/dirdlg.h>
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>
#include <wx/textdlg.h>
#include <wx/toolbar.h>
#include <wx/msgdlg.h>

EditorFrame::EditorFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Powder Editor", pos, size)
{
	this->auiManager = new wxAuiManager(this, wxAUI_MGR_DEFAULT);
	this->panelMenuCount = 0;

	this->MakePanels();

	const int entryCount = 8;
	wxAcceleratorEntry entries[entryCount];
	entries[0].Set(wxACCEL_CTRL, 'N', ID_New);
	entries[1].Set(wxACCEL_CTRL, 'S', ID_Save);
	entries[2].Set(wxACCEL_CTRL, 'O', ID_Open);
	entries[3].Set(wxACCEL_NORMAL, WXK_F10, ID_StepOver);
	entries[4].Set(wxACCEL_NORMAL, WXK_F11, ID_StepInto);
	entries[5].Set(wxACCEL_SHIFT, WXK_F11, ID_StepOut);
	entries[6].Set(wxACCEL_NORMAL, WXK_F5, ID_RunWithoutDebugger);
	entries[7].Set(wxACCEL_CTRL, WXK_F5, ID_RunWithDebugger);

	wxAcceleratorTable table(entryCount, entries);
	this->SetAcceleratorTable(table);

	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_New, "New", "Create a new blank source file."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Save, "Save", "Save the currently shown source file, if any."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Open, "Open", "Browse to and open a source file."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Close, "Close", "Close the currently shown source file, if any."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SaveAll, "Save All", "Save all currently open files."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_CloseAll, "Close All", "Close all currently open files."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_OpenDirectory, "Open Directory", "Open a given directory to work with and edit all powder files in that directory."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_CloseDirectory, "Close Directory", "Close the currently open directory, if any."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_DeleteExecutables, "Delete Executables", "Delete all compilation targets in the project directory."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Go skiing."));

	fileMenu->FindItem(ID_New)->SetAccel(&entries[0]);
	fileMenu->FindItem(ID_Save)->SetAccel(&entries[1]);
	fileMenu->FindItem(ID_Open)->SetAccel(&entries[2]);

	fileMenu->FindItem(ID_Exit)->SetBitmap(wxArtProvider::GetBitmap(wxART_QUIT));
	fileMenu->FindItem(ID_New)->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW, wxART_MENU, wxSize(16, 16)));
	fileMenu->FindItem(ID_Save)->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_MENU, wxSize(16, 16)));
	fileMenu->FindItem(ID_Open)->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_MENU, wxSize(16, 16)));
	fileMenu->FindItem(ID_DeleteExecutables)->SetBitmap(wxArtProvider::GetBitmap(ART_EDITOR_DELETE_TARGETS, wxART_MENU));

	wxMenu* runMenu = new wxMenu();
	runMenu->Append(new wxMenuItem(runMenu, ID_RunWithDebugger, "Run with Debugger", "Run the currently shown script with the debugger attached."));
	runMenu->Append(new wxMenuItem(runMenu, ID_RunWithoutDebugger, "Run without Debugger", "Run the currently shown script without a debugger attached."));
	runMenu->AppendSeparator();
	runMenu->Append(new wxMenuItem(runMenu, ID_StepOver, "Step Over", "Step over the current statement of function call."));
	runMenu->Append(new wxMenuItem(runMenu, ID_StepInto, "Step Into", "Step into the current function call, if any; or just step over."));
	runMenu->Append(new wxMenuItem(runMenu, ID_StepOut, "Step Out", "Step out of the current function call, if any; or just resume."));
	runMenu->AppendSeparator();
	runMenu->Append(new wxMenuItem(runMenu, ID_PauseScript, "Pause Script", "Suspend the currently running script as soon as possible."));
	runMenu->Append(new wxMenuItem(runMenu, ID_ResumeScript, "Resume Script", "Resume execution of the currently suspended script, if any."));
	runMenu->Append(new wxMenuItem(runMenu, ID_KillScript, "Kill Script", "Prematurely end the currently running script, if any."));

	runMenu->FindItem(ID_StepOver)->SetAccel(&entries[3]);
	runMenu->FindItem(ID_StepInto)->SetAccel(&entries[4]);
	runMenu->FindItem(ID_StepOut)->SetAccel(&entries[5]);
	runMenu->FindItem(ID_RunWithDebugger)->SetAccel(&entries[6]);
	runMenu->FindItem(ID_RunWithoutDebugger)->SetAccel(&entries[7]);

	runMenu->FindItem(ID_StepOver)->SetBitmap(wxArtProvider::GetBitmap(ART_EDITOR_STEP_OVER, wxART_MENU));
	runMenu->FindItem(ID_StepInto)->SetBitmap(wxArtProvider::GetBitmap(ART_EDITOR_STEP_INTO, wxART_MENU));
	runMenu->FindItem(ID_StepOut)->SetBitmap(wxArtProvider::GetBitmap(ART_EDITOR_STEP_OUT, wxART_MENU));
	runMenu->FindItem(ID_RunWithDebugger)->SetBitmap(wxArtProvider::GetBitmap(ART_EDITOR_RUN_WITH_DEBUGGER, wxART_MENU));
	runMenu->FindItem(ID_RunWithoutDebugger)->SetBitmap(wxArtProvider::GetBitmap(ART_EDITOR_RUN_WITHOUT_DEBUGGER, wxART_MENU));
	runMenu->FindItem(ID_PauseScript)->SetBitmap(wxArtProvider::GetBitmap(ART_EDITOR_PAUSE_SCRIPT, wxART_MENU));
	runMenu->FindItem(ID_ResumeScript)->SetBitmap(wxArtProvider::GetBitmap(ART_EDITOR_RESUME_SCRIPT, wxART_MENU));
	runMenu->FindItem(ID_KillScript)->SetBitmap(wxArtProvider::GetBitmap(ART_EDITOR_KILL_SCRIPT, wxART_MENU));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about box."));

	helpMenu->FindItem(ID_About)->SetBitmap(wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_MENU, wxSize(16, 16)));

	wxMenu* panelsMenu = this->MakePanelsMenu();

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(runMenu, "Run");
	menuBar->Append(panelsMenu, "Panels");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->SetStatusBar(new wxStatusBar(this));
	this->GetStatusBar()->SetFieldsCount(2);

	wxToolBar* toolBar = this->CreateToolBar();

	toolBar->AddTool(ID_New, "New", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR, wxSize(16, 16)));
	toolBar->AddTool(ID_Save, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, wxSize(16, 16)));
	toolBar->AddTool(ID_Open, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, wxSize(16, 16)));
	//toolBar->AddTool(ID_Close, "Close", ...
	toolBar->AddSeparator();
	toolBar->AddTool(ID_DeleteExecutables, "Delete Executables", wxArtProvider::GetBitmap(ART_EDITOR_DELETE_TARGETS, wxART_TOOLBAR));
	toolBar->AddSeparator();
	toolBar->AddTool(ID_RunWithDebugger, "Run with Debugger", wxArtProvider::GetBitmap(ART_EDITOR_RUN_WITH_DEBUGGER, wxART_TOOLBAR));
	toolBar->AddTool(ID_RunWithoutDebugger, "Run without Debugger", wxArtProvider::GetBitmap(ART_EDITOR_RUN_WITHOUT_DEBUGGER, wxART_TOOLBAR));
	toolBar->AddSeparator();
	toolBar->AddTool(ID_StepOver, "Step Over", wxArtProvider::GetBitmap(ART_EDITOR_STEP_OVER, wxART_TOOLBAR));
	toolBar->AddTool(ID_StepInto, "Step Into", wxArtProvider::GetBitmap(ART_EDITOR_STEP_INTO, wxART_TOOLBAR));
	toolBar->AddTool(ID_StepOut, "Step Out", wxArtProvider::GetBitmap(ART_EDITOR_STEP_OUT, wxART_TOOLBAR));
	toolBar->AddSeparator();
	toolBar->AddTool(ID_PauseScript, "Pause Script", wxArtProvider::GetBitmap(ART_EDITOR_PAUSE_SCRIPT, wxART_TOOLBAR));
	toolBar->AddTool(ID_ResumeScript, "Resume Script", wxArtProvider::GetBitmap(ART_EDITOR_RESUME_SCRIPT, wxART_TOOLBAR));
	toolBar->AddTool(ID_KillScript, "Kill Script", wxArtProvider::GetBitmap(ART_EDITOR_KILL_SCRIPT, wxART_TOOLBAR));

	toolBar->Realize();

	this->Bind(wxEVT_MENU, &EditorFrame::OnNewFile, this, ID_New);
	this->Bind(wxEVT_MENU, &EditorFrame::OnSaveFile, this, ID_Save);
	this->Bind(wxEVT_MENU, &EditorFrame::OnOpenFile, this, ID_Open);
	this->Bind(wxEVT_MENU, &EditorFrame::OnCloseFile, this, ID_Close);
	this->Bind(wxEVT_MENU, &EditorFrame::OnSaveAllFiles, this, ID_SaveAll);
	this->Bind(wxEVT_MENU, &EditorFrame::OnCloseAllFiles, this, ID_CloseAll);
	this->Bind(wxEVT_MENU, &EditorFrame::OnOpenDirectory, this, ID_OpenDirectory);
	this->Bind(wxEVT_MENU, &EditorFrame::OnCloseDirectory, this, ID_CloseDirectory);
	this->Bind(wxEVT_MENU, &EditorFrame::OnDeleteExecutables, this, ID_DeleteExecutables);
	this->Bind(wxEVT_MENU, &EditorFrame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &EditorFrame::OnRunWithDebugger, this, ID_RunWithDebugger);
	this->Bind(wxEVT_MENU, &EditorFrame::OnRunWithoutDebugger, this, ID_RunWithoutDebugger);
	this->Bind(wxEVT_MENU, &EditorFrame::OnPauseScript, this, ID_PauseScript);
	this->Bind(wxEVT_MENU, &EditorFrame::OnResumeScript, this, ID_ResumeScript);
	this->Bind(wxEVT_MENU, &EditorFrame::OnKillScript, this, ID_KillScript);
	this->Bind(wxEVT_MENU, &EditorFrame::OnStepOver, this, ID_StepOver);
	this->Bind(wxEVT_MENU, &EditorFrame::OnStepInto, this, ID_StepInto);
	this->Bind(wxEVT_MENU, &EditorFrame::OnStepOut, this, ID_StepOut);
	this->Bind(wxEVT_MENU, &EditorFrame::OnAbout, this, ID_About);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_New);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_Save);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_Open);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_Close);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_SaveAll);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseAll);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_OpenDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_DeleteExecutables);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_RunWithDebugger);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_RunWithoutDebugger);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_PauseScript);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_ResumeScript);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_KillScript);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_StepOver);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_StepInto);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_StepOut);
	this->Bind(wxEVT_CLOSE_WINDOW, &EditorFrame::OnClose, this);
	this->Bind(EVT_RUNTHREAD_ENTERING, &EditorFrame::OnRunThreadEntering, this);
	this->Bind(EVT_RUNTHREAD_EXITING, &EditorFrame::OnRunThreadExiting, this);
	this->Bind(EVT_RUNTHREAD_EXCEPTION, &EditorFrame::OnRunThreadException, this);
	this->Bind(EVT_RUNTHREAD_OUTPUT, &EditorFrame::OnRunThreadOutput, this);
	this->Bind(EVT_RUNTHREAD_INPUT, &EditorFrame::OnRunThreadInput, this);
	this->Bind(EVT_RUNTHREAD_SUSPENDED, &EditorFrame::OnRunThreadSuspended, this);
	this->Bind(EVT_TERMINAL_INPUT_READY, &EditorFrame::OnTerminalInputReady, this);

	this->NotifyPanels(Panel::APP_OPENING, nullptr);

	this->auiManager->Update();
}

/*virtual*/ EditorFrame::~EditorFrame()
{
	this->auiManager->UnInit();
	delete this->auiManager;
}

wxMenu* EditorFrame::MakePanelsMenu()
{
	wxMenu* panelsMenu = new wxMenu();

	wxAuiPaneInfoArray& paneInfoArray = auiManager->GetAllPanes();
	int j = 0;
	for (int i = 0; i < (signed)paneInfoArray.GetCount(); i++)
	{
		if (paneInfoArray[i].HasCloseButton())
		{
			Panel* panel = wxDynamicCast(paneInfoArray[i].window, Panel);
			if (panel)
			{
				wxClassInfo* classInfo = panel->GetClassInfo();
				panel->menuItemId = ID_PANEL_BASE + j;

				wxMenuItem* menuItem = new wxMenuItem(panelsMenu, panel->menuItemId, paneInfoArray[i].name, wxEmptyString, wxITEM_CHECK);
				panelsMenu->Append(menuItem);

				this->Bind(wxEVT_MENU, &EditorFrame::OnPanelMenuItemClicked, this);
				this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, panel->menuItemId);

				j++;
			}
		}
	}

	this->panelMenuCount = j;

	return panelsMenu;
}

void EditorFrame::OnPanelMenuItemClicked(wxCommandEvent& event)
{
	wxAuiPaneInfo* paneInfo = nullptr;
	bool shown = this->IsPanelShown(event.GetId(), &paneInfo);
	if (paneInfo)
	{
		if (shown)
			paneInfo->Hide();
		else
			paneInfo->Show();
		this->auiManager->Update();
	}
}

void EditorFrame::MakePanels(void)
{
	wxClassInfo* basePanelClassInfo = wxClassInfo::FindClass("Panel");
	if (!basePanelClassInfo)
		return;

	const wxClassInfo* classInfo = wxClassInfo::GetFirst();
	while (classInfo)
	{
		if (classInfo != basePanelClassInfo && classInfo->IsKindOf(basePanelClassInfo))
		{
			Panel* panel = (Panel*)classInfo->CreateObject();
			bool created = panel->Create(this);
			if (!created)
				delete panel;
			else if (!panel->MakeControls())
				delete panel;
			else
			{
				wxAuiPaneInfo paneInfo;
				panel->GetPaneInfo(paneInfo);
				bool paneAdded = auiManager->AddPane(panel, paneInfo);
				if (!paneAdded)
				{
					//...
				}
			}
		}
		classInfo = classInfo->GetNext();
	}
}

void EditorFrame::NotifyPanels(Panel::Notification notification, void* notifyData)
{
	wxAuiPaneInfoArray& paneInfoArray = auiManager->GetAllPanes();
	for (int i = 0; i < (signed)paneInfoArray.GetCount(); i++)
	{
		Panel* panel = wxDynamicCast(paneInfoArray[i].window, Panel);
		if (panel)
			panel->OnNotified(notification, notifyData);
	}
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

void EditorFrame::OnNewFile(wxCommandEvent& event)
{
	wxTextEntryDialog textDialog(this, "Please enter source file name.", "New File");
	if (wxID_OK == textDialog.ShowModal())
	{
		wxFileName fileName(wxGetApp().GetProjectDirectory() + "/" + textDialog.GetValue());
		if (fileName.GetExt().size() == 0)
			fileName.SetExt("pow");

		SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
		if (sourceFilePanel)
		{
			if (sourceFilePanel->notebookControl->NewSourceFile(fileName.GetFullPath()))
			{
				this->NotifyPanels(Panel::DIRECTORY_CHANGED, nullptr);
			}
		}
	}
}

void EditorFrame::OnSaveFile(wxCommandEvent& event)
{
	SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
	if (sourceFilePanel)
	{
		int pageNumber = sourceFilePanel->notebookControl->GetSelection();
		if (pageNumber >= 0)
			sourceFilePanel->notebookControl->SaveSourceFile(pageNumber);
	}
}

void EditorFrame::OnOpenFile(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, "Open Powder Source File", "", "", "Powder File (*.pow)|*.pow", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (wxID_OK == openFileDialog.ShowModal())
	{
		SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
		if (sourceFilePanel)
			sourceFilePanel->notebookControl->OpenSourceFile(openFileDialog.GetPath());
	}
}

void EditorFrame::OnCloseFile(wxCommandEvent& event)
{
	SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
	if (sourceFilePanel)
	{
		int pageNumber = sourceFilePanel->notebookControl->GetSelection();
		if (pageNumber >= 0)
			sourceFilePanel->notebookControl->CloseSourceFile(pageNumber);
	}
}

void EditorFrame::OnSaveAllFiles(wxCommandEvent& event)
{
	SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
	if (sourceFilePanel)
		sourceFilePanel->notebookControl->SaveAllFiles();
}

void EditorFrame::OnCloseAllFiles(wxCommandEvent& event)
{
	SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
	if (sourceFilePanel)
		sourceFilePanel->notebookControl->CloseAllFiles();
}

void EditorFrame::OnOpenDirectory(wxCommandEvent& event)
{
	wxDirDialog dirDialog(this, "Open which directory?", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (wxID_OK == dirDialog.ShowModal())
	{
		wxGetApp().SetProjectDirectory(dirDialog.GetPath());
		this->NotifyPanels(Panel::DIRECTORY_OPENED, nullptr);
	}
}

void EditorFrame::OnCloseDirectory(wxCommandEvent& event)
{
	SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
	if (sourceFilePanel)
	{
		if (sourceFilePanel->notebookControl->CloseAllFiles())
		{
			this->NotifyPanels(Panel::DIRECTORY_CLOSED, nullptr);
			wxGetApp().SetProjectDirectory("");
		}
	}
}

void EditorFrame::OnDeleteExecutables(wxCommandEvent& event)
{
	wxDir projectDir(wxGetApp().GetProjectDirectory());
	if (!projectDir.IsOpened())
		wxMessageBox("Failed to open project directory: " + wxGetApp().GetProjectDirectory(), "Error!", wxICON_ERROR | wxOK, this);
	else
	{
		FileFinder fileFinder;
		fileFinder.extensionArray.Add("pwx");
		fileFinder.extensionArray.Add("debuginfo");
		projectDir.Traverse(fileFinder, wxEmptyString, wxDIR_DEFAULT);
		if (fileFinder.foundFileArray.size() == 0)
			wxMessageBox("No files found to delete!", "Success!", wxICON_INFORMATION | wxOK, this);
		else
		{
			wxString errorMsg;

			for (int i = 0; i < (signed)fileFinder.foundFileArray.size(); i++)
			{
				wxString doomedFile = fileFinder.foundFileArray[i];
				if (!wxRemoveFile(doomedFile))
					errorMsg += "Failed to delete file: " + doomedFile + "\n";
			}

			if (errorMsg.length() > 0)
				wxMessageBox("Some files could not be deleted!\n\n" + errorMsg, "Error!", wxICON_ERROR | wxOK, this);
			else
				wxMessageBox(wxString::Format("Deleted %d files!", fileFinder.foundFileArray.size()), "Success!", wxICON_INFORMATION | wxOK, this);
		}		
	}
}

void EditorFrame::OnRunWithDebugger(wxCommandEvent& event)
{
	this->KickoffRunThread(true);
	this->UpdateWindowTitlebarText();
}

void EditorFrame::OnRunWithoutDebugger(wxCommandEvent& event)
{
	this->KickoffRunThread(false);
	this->UpdateWindowTitlebarText();
}

void EditorFrame::KickoffRunThread(bool debuggingEnabled)
{
	if (!wxGetApp().GetRunThread())
	{
		SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
		if (sourceFilePanel)
		{
			SourceFileEditControl* editControl = sourceFilePanel->notebookControl->GetSelectedEditControl();
			if (editControl)
			{
				RunThread* runThread = new RunThread(editControl->filePath.GetFullPath(), this, debuggingEnabled);
				wxGetApp().SetRunThread(runThread);
				runThread->Run();
			}
		}
	}
}

void EditorFrame::OnPauseScript(wxCommandEvent& event)
{
	wxGetApp().GetRunThread()->MainThread_Pause();
	this->UpdateWindowTitlebarText();
}

void EditorFrame::OnResumeScript(wxCommandEvent& event)
{
	wxGetApp().GetRunThread()->MainThread_Resume();
	this->UpdateWindowTitlebarText();
}

void EditorFrame::OnKillScript(wxCommandEvent& event)
{
	wxGetApp().GetRunThread()->MainThread_ExitNow();
	this->UpdateWindowTitlebarText();
}

void EditorFrame::UpdateWindowTitlebarText(void)
{
	wxString label = "Powder Editor";
	if (wxGetApp().GetRunThread())
	{
		wxFileName fileName = wxGetApp().GetRunThread()->sourceFilePath;
		label += " -- Running: " + fileName.GetName() + "." + fileName.GetExt();
		switch (wxGetApp().GetRunThread()->suspensionState)
		{
		case RunThread::NOT_SUSPENDED:
			label += " [RUNNING]";
			break;
		case RunThread::SUSPENDED_FOR_DEBUG:
		case RunThread::SUSPENDED_FOR_INPUT:
			label += " [SUSPENDED]";
			break;
		}
	}

	this->SetLabel(label);
}

void EditorFrame::OnRunThreadEntering(wxThreadEvent& event)
{
	this->UpdateWindowTitlebarText();
	this->NotifyPanels(Panel::RUNTHREAD_STARTED, nullptr);
}

void EditorFrame::OnRunThreadExiting(wxThreadEvent& event)
{
	RunThread* runThread = wxGetApp().GetRunThread();
	if (runThread)
	{
		runThread->Wait(wxThreadWait::wxTHREAD_WAIT_BLOCK);
		this->NotifyPanels(Panel::RUNTHREAD_ENDED, nullptr);
		delete runThread;
		wxGetApp().SetRunThread(nullptr);
	}

	this->UpdateWindowTitlebarText();
}

void EditorFrame::OnRunThreadOutput(RunThreadOutputEvent& event)
{
	TerminalPanel* terminalPanel = this->FindPanel<TerminalPanel>("Terminal");
	if (terminalPanel)
		terminalPanel->terminalControl->AppendText(event.outputText);
}

void EditorFrame::OnRunThreadInput(RunThreadInputEvent& event)
{
	TerminalPanel* terminalPanel = this->FindPanel<TerminalPanel>("Terminal");
	if (terminalPanel)
		terminalPanel->terminalControl->EditString(event.inputText);
}

void EditorFrame::OnTerminalInputReady(wxCommandEvent& event)
{
	if (wxGetApp().GetRunThread())
		wxGetApp().GetRunThread()->MainThread_Resume();
}

void EditorFrame::OnRunThreadException(RunThreadExceptionEvent& event)
{
	TerminalPanel* terminalPanel = this->FindPanel<TerminalPanel>("Terminal");
	if (terminalPanel)
	{
		terminalPanel->terminalControl->AppendText("-------------------- ERROR --------------------\n");
		terminalPanel->terminalControl->AppendText(event.errorMsg + "\n");
		terminalPanel->terminalControl->AppendText("-------------------- ERROR --------------------\n");
	}
}

void EditorFrame::OnRunThreadSuspended(RunThreadSuspendedEvent& event)
{
	this->NotifyPanels(Panel::RUNTHREAD_SUSPENDED, (void*)&event);
	this->UpdateWindowTitlebarText();
}

void EditorFrame::OnStepOver(wxCommandEvent& event)
{
	wxGetApp().GetRunThread()->MainThread_StepOver();
}

void EditorFrame::OnStepInto(wxCommandEvent& event)
{
	wxGetApp().GetRunThread()->MainThread_StepInto();
}

void EditorFrame::OnStepOut(wxCommandEvent& event)
{
	wxGetApp().GetRunThread()->MainThread_StepOut();
}

bool EditorFrame::IsPanelShown(int panelMenuId, wxAuiPaneInfo** foundPaneInfo /*= nullptr*/)
{
	if (foundPaneInfo)
		*foundPaneInfo = nullptr;
	wxAuiPaneInfoArray& paneInfoArray = auiManager->GetAllPanes();
	for (int i = 0; i < (signed)paneInfoArray.GetCount(); i++)
	{
		wxAuiPaneInfo& paneInfo = paneInfoArray[i];
		Panel* panel = wxDynamicCast(paneInfo.window, Panel);
		if (panel && panel->menuItemId == panelMenuId)
		{
			if (foundPaneInfo)
				*foundPaneInfo = &paneInfoArray[i];
			return paneInfo.IsShown();
		}
	}
	return false;
}

void EditorFrame::OnUpdateMenuItemUI(wxUpdateUIEvent& event)
{
	int id = event.GetId();

	int panelIdMin = ID_PANEL_BASE;
	int panelIdMax = ID_PANEL_BASE + this->panelMenuCount - 1;
	if (panelIdMin <= id && id <= panelIdMax)
		event.Check(this->IsPanelShown(id));
	else
	{
		switch (id)
		{
			case ID_New:
			{
				// TODO: Write this.
				break;
			}
			case ID_Save:
			{
				SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
				if (sourceFilePanel)
				{
					if (sourceFilePanel->notebookControl->OpenFileCount() == 0)
						event.Enable(false);
					else
						event.Enable(sourceFilePanel->notebookControl->GetSelectedEditControl()->modified);
				}
				break;
			}
			case ID_Open:
			{
				event.Enable(true);
				break;
			}
			case ID_Close:
			{
				SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
				if (sourceFilePanel)
					event.Enable(sourceFilePanel->notebookControl->GetSelectedEditControl() ? true : false);
				break;
			}
			case ID_SaveAll:
			{
				SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
				if (sourceFilePanel)
					event.Enable(sourceFilePanel->notebookControl->AnyFilesModified());
				break;
			}
			case ID_CloseAll:
			{
				SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
				if (sourceFilePanel)
					event.Enable(sourceFilePanel->notebookControl->OpenFileCount() > 0);
				break;
			}
			case ID_OpenDirectory:
			{
				event.Enable(wxGetApp().GetProjectDirectory().IsEmpty());
				break;
			}
			case ID_DeleteExecutables:
			case ID_CloseDirectory:
			{
				event.Enable(!wxGetApp().GetProjectDirectory().IsEmpty());
				break;
			}
			case ID_RunWithDebugger:
			case ID_RunWithoutDebugger:
			{
				if (wxGetApp().GetRunThread())
					event.Enable(false);
				else
				{
					SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
					if (sourceFilePanel)
					{
						SourceFileEditControl* editControl = sourceFilePanel->notebookControl->GetSelectedEditControl();
						event.Enable(editControl != nullptr);
						if (editControl)
						{
							if (event.GetId() == ID_RunWithDebugger)
								event.SetText("Run " + editControl->GetFileName() + " with Debugger\tCtrl+F5");
							else
								event.SetText("Run " + editControl->GetFileName() + " without Debugger\tF5");
						}
						else
						{
							if (event.GetId() == ID_RunWithDebugger)
								event.SetText("Run with Debugger\tCtrl+F5");
							else
								event.SetText("Run without Debugger\tF5");
						}
					}
				}

				break;
			}
			case ID_PauseScript:
			{
				if (!wxGetApp().GetRunThread())
					event.Enable(false);
				else
					event.Enable(wxGetApp().GetRunThread()->suspensionState == RunThread::NOT_SUSPENDED && wxGetApp().GetRunThread()->debuggingEnabled);
				break;
			}
			case ID_ResumeScript:
			case ID_StepOver:
			case ID_StepInto:
			case ID_StepOut:
			{
				if (!wxGetApp().GetRunThread())
					event.Enable(false);
				else
					event.Enable(wxGetApp().GetRunThread()->suspensionState == RunThread::SUSPENDED_FOR_DEBUG && wxGetApp().GetRunThread()->debuggingEnabled);
				break;
			}
			case ID_KillScript:
			{
				event.Enable(wxGetApp().GetRunThread() != nullptr);
				break;
			}
		}
	}
}

void EditorFrame::SaveWindowAdjustments()
{
	wxConfig* config = wxGetApp().GetConfig();

	config->Write("windowX", this->GetPosition().x);
	config->Write("windowY", this->GetPosition().y);
	config->Write("windowWidth", this->GetSize().x);
	config->Write("windowHeight", this->GetSize().y);
	
	wxString perspective = this->auiManager->SavePerspective();
	config->Write("auiManagerPerspective", perspective);
}

void EditorFrame::RestoreWindowAdjustments()
{
	wxConfig* config = wxGetApp().GetConfig();

	// TODO: May want command-line argument that, if given, causes us to ignore configs at start-up.

	wxPoint point;
	point.x = config->Read("windowX", -1);
	point.y = config->Read("windowY", -1);
	if (point.x >= 0 && point.y >= 0)
		this->SetPosition(point);

	wxSize size;
	size.x = config->Read("windowWidth", -1);
	size.y = config->Read("windowHeight", -1);
	if (size.x >= 0 && size.y >= 0)
		this->SetSize(size);

	wxString perspective;
	if (config->Read("auiManagerPerspective", &perspective))
		this->auiManager->LoadPerspective(perspective);
}

void EditorFrame::OnClose(wxCloseEvent& event)
{
	this->SaveWindowAdjustments();
	this->NotifyPanels(Panel::APP_CLOSING, nullptr);

	SourceFilePanel* sourceFilePanel = this->FindPanel<SourceFilePanel>("SourceFile");
	if (sourceFilePanel && !sourceFilePanel->notebookControl->CloseAllFiles())
		return;

	RunThread* runThread = wxGetApp().GetRunThread();
	if (runThread)
	{
		wxGetApp().SetRunThread(nullptr);
		this->Unbind(EVT_RUNTHREAD_EXITING, &EditorFrame::OnRunThreadExiting, this);
		runThread->MainThread_ExitNow();
		runThread->Wait(wxTHREAD_WAIT_BLOCK);
		delete runThread;
	}

	wxFrame::OnCloseWindow(event);
}