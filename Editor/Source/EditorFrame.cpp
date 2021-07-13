#include "EditorFrame.h"
#include "DirectoryTreeControl.h"
#include "SourceFileNotebookControl.h"
#include "SourceFileEditControl.h"
#include "TerminalControl.h"
#include "RunThread.h"
#include "EditorApp.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/dirdlg.h>
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>

EditorFrame::EditorFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Powder Editor", pos, size)
{
	const int entryCount = 5;
	wxAcceleratorEntry entries[entryCount];
	entries[0].Set(wxACCEL_CTRL, 'S', ID_Save);
	entries[1].Set(wxACCEL_CTRL, 'O', ID_Open);
	entries[2].Set(wxACCEL_NORMAL, WXK_F10, ID_StepOver);
	entries[3].Set(wxACCEL_NORMAL, WXK_F11, ID_StepInto);
	entries[4].Set(wxACCEL_SHIFT, WXK_F11, ID_StepOut);

	wxAcceleratorTable table(entryCount, entries);
	this->SetAcceleratorTable(table);

	wxMenu* fileMenu = new wxMenu();
	wxMenuItem* saveFileMenuItem = new wxMenuItem(fileMenu, ID_Save, "Save", "Save the currently shown source file, if any.");
	wxMenuItem* openFileMenuItem = new wxMenuItem(fileMenu, ID_Open, "Open", "Browse to and open a source file.");
	wxMenuItem* closeFileMenuItem = new wxMenuItem(fileMenu, ID_Close, "Close", "Close the currently shown source file, if any.");
	saveFileMenuItem->SetAccel(&entries[0]);
	openFileMenuItem->SetAccel(&entries[1]);
	fileMenu->Append(saveFileMenuItem);
	fileMenu->Append(openFileMenuItem);
	fileMenu->Append(closeFileMenuItem);
	fileMenu->AppendSeparator();
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
	runMenu->AppendSeparator();
	wxMenuItem* stepOverMenuItem = new wxMenuItem(runMenu, ID_StepOver, "Step Over", "Step over the current statement of function call.");
	wxMenuItem* stepIntoMenuItem = new wxMenuItem(runMenu, ID_StepInto, "Step Into", "Step into the current function call, if any; or just step over.");
	wxMenuItem* stepOutMenuItem = new wxMenuItem(runMenu, ID_StepOut, "Step Out", "Step out of the current function call, if any; or just resume.");
	stepOverMenuItem->SetAccel(&entries[2]);
	stepIntoMenuItem->SetAccel(&entries[3]);
	stepOutMenuItem->SetAccel(&entries[4]);
	runMenu->Append(stepOverMenuItem);
	runMenu->Append(stepIntoMenuItem);
	runMenu->Append(stepOutMenuItem);
	runMenu->AppendSeparator();
	runMenu->Append(new wxMenuItem(runMenu, ID_PauseScript, "Pause Script", "Suspend the currently running script as soon as possible."));
	runMenu->Append(new wxMenuItem(runMenu, ID_ResumeScript, "Resume Script", "Resume execution of the currently suspended script, if any."));
	runMenu->Append(new wxMenuItem(runMenu, ID_KillScript, "Kill Script", "Prematurely end the currently running script, if any."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(runMenu, "Run");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->SetStatusBar(new wxStatusBar(this));
	this->GetStatusBar()->SetFieldsCount(2);

	// TODO: Need a way to create a new source file under any directory of the currently open project directory.
	this->Bind(wxEVT_MENU, &EditorFrame::OnSaveFile, this, ID_Save);
	this->Bind(wxEVT_MENU, &EditorFrame::OnOpenFile, this, ID_Open);
	this->Bind(wxEVT_MENU, &EditorFrame::OnCloseFile, this, ID_Close);
	this->Bind(wxEVT_MENU, &EditorFrame::OnSaveAllFiles, this, ID_SaveAll);
	this->Bind(wxEVT_MENU, &EditorFrame::OnCloseAllFiles, this, ID_CloseAll);
	this->Bind(wxEVT_MENU, &EditorFrame::OnOpenDirectory, this, ID_OpenDirectory);
	this->Bind(wxEVT_MENU, &EditorFrame::OnCloseDirectory, this, ID_CloseDirectory);
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
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_Save);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_Open);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_Close);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_SaveAll);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseAll);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_OpenDirectory);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_CloseDirectory);
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

	this->verticalSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D);
	this->directoryTreeControl = new DirectoryTreeControl(this->verticalSplitter);
	this->horizontalSplitter = new wxSplitterWindow(this->verticalSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D);
	this->sourceFileNotebookControl = new SourceFileNotebookControl(this->horizontalSplitter);
	this->sourceFileNotebookControl->RestoreOpenFiles();
	this->terminalControl = new TerminalControl(this->horizontalSplitter);
	this->horizontalSplitter->SplitHorizontally(this->sourceFileNotebookControl, this->terminalControl);
	this->verticalSplitter->SplitVertically(this->directoryTreeControl, this->horizontalSplitter);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(verticalSplitter, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);

	verticalSplitter->SetSashPosition(200);
	horizontalSplitter->SetSashPosition(300);

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

void EditorFrame::OnSaveFile(wxCommandEvent& event)
{
	int pageNumber = this->sourceFileNotebookControl->GetSelection();
	if (pageNumber >= 0)
		this->sourceFileNotebookControl->SaveSourceFile(pageNumber);
}

void EditorFrame::OnOpenFile(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, "Open Powder Source File", "", "", "Powder File (*.pow)|*.pow", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_OK)
		this->sourceFileNotebookControl->OpenSourceFile(openFileDialog.GetPath());
}

void EditorFrame::OnCloseFile(wxCommandEvent& event)
{
	int pageNumber = this->sourceFileNotebookControl->GetSelection();
	if (pageNumber >= 0)
		this->sourceFileNotebookControl->CloseSourceFile(pageNumber);
}

void EditorFrame::OnSaveAllFiles(wxCommandEvent& event)
{
	this->sourceFileNotebookControl->SaveAllFiles();
}

void EditorFrame::OnCloseAllFiles(wxCommandEvent& event)
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
	this->KickoffRunThread(true);
}

void EditorFrame::OnRunWithoutDebugger(wxCommandEvent& event)
{
	this->KickoffRunThread(false);
}

void EditorFrame::KickoffRunThread(bool debuggingEnabled)
{
	if (!wxGetApp().GetRunThread())
	{
		SourceFileEditControl* editControl = this->sourceFileNotebookControl->GetSelectedEditControl();
		if (editControl)
		{
			RunThread* runThread = new RunThread(editControl->filePath.GetFullPath(), this, debuggingEnabled);
			wxGetApp().SetRunThread(runThread);
			runThread->Run();
		}
	}
}

void EditorFrame::OnPauseScript(wxCommandEvent& event)
{
	wxGetApp().GetRunThread()->MainThread_Pause();
}

void EditorFrame::OnResumeScript(wxCommandEvent& event)
{
	wxGetApp().GetRunThread()->MainThread_Resume();
}

void EditorFrame::OnKillScript(wxCommandEvent& event)
{
	wxGetApp().GetRunThread()->MainThread_ExitNow();
}

void EditorFrame::OnRunThreadEntering(wxThreadEvent& event)
{
	this->terminalControl->Clear();
}

void EditorFrame::OnRunThreadExiting(wxThreadEvent& event)
{
	RunThread* runThread = wxGetApp().GetRunThread();
	if (runThread)
	{
		runThread->Wait(wxThreadWait::wxTHREAD_WAIT_BLOCK);
		this->terminalControl->AppendText(wxString::Format("\n\nExecution time: %d ms", runThread->executionTimeMilliseconds));
		delete runThread;
		wxGetApp().SetRunThread(nullptr);
	}

	this->sourceFileNotebookControl->ClearExecutionMarkers();
}

void EditorFrame::OnRunThreadOutput(RunThreadOutputEvent& event)
{
	this->terminalControl->AppendText(event.outputText);
}

void EditorFrame::OnRunThreadInput(RunThreadInputEvent& event)
{
	this->terminalControl->EditString(event.inputText);
}

void EditorFrame::OnTerminalInputReady(wxCommandEvent& event)
{
	if (wxGetApp().GetRunThread())
		wxGetApp().GetRunThread()->MainThread_Resume();
}

void EditorFrame::OnRunThreadException(RunThreadExceptionEvent& event)
{
	this->terminalControl->AppendText("-------------------- ERROR --------------------\n");
	this->terminalControl->AppendText(event.errorMsg);
	this->terminalControl->AppendText("-------------------- ERROR --------------------\n");
}

void EditorFrame::OnRunThreadSuspended(RunThreadSuspendedEvent& event)
{
	if (this->sourceFileNotebookControl->OpenSourceFile(event.sourceFile))
	{
		SourceFileEditControl* editControl = this->sourceFileNotebookControl->FindEditControl(event.sourceFile);
		if (editControl)
			editControl->ShowExecutionSuspendedAt(event.lineNumber, event.columnNumber);
	}
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

void EditorFrame::OnUpdateMenuItemUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_Save:
		{
			if (this->sourceFileNotebookControl->OpenFileCount() == 0)
				event.Enable(false);
			else
				event.Enable(this->sourceFileNotebookControl->GetSelectedEditControl()->modified);
			break;
		}
		case ID_Open:
		{
			event.Enable(true);
			break;
		}
		case ID_Close:
		{
			event.Enable(this->sourceFileNotebookControl->GetSelectedEditControl() ? true : false);
			break;
		}
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
			if (wxGetApp().GetRunThread())
				event.Enable(false);
			else
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
			}

			break;
		}
		case ID_PauseScript:
		{
			if (!wxGetApp().GetRunThread())
				event.Enable(false);
			else
				event.Enable(wxGetApp().GetRunThread()->suspensionState == RunThread::NOT_SUSPENDED);
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
				event.Enable(wxGetApp().GetRunThread()->suspensionState == RunThread::SUSPENDED_FOR_DEBUG);
			break;
		}
		case ID_KillScript:
		{
			event.Enable(wxGetApp().GetRunThread() ? true : false);
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
	wxGetApp().GetConfig()->Write("horizontalSplitterSashPos", this->horizontalSplitter->GetSashPosition());
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

	sashPos = wxGetApp().GetConfig()->Read("horizontalSplitterSashPos", -1);
	if (sashPos >= 0)
		this->horizontalSplitter->SetSashPosition(sashPos);
}

void EditorFrame::OnClose(wxCloseEvent& event)
{
	this->SaveWindowAdjustments();
	this->sourceFileNotebookControl->RememberCurrentlyOpenFiles();

	if (!this->sourceFileNotebookControl->CloseAllFiles())
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