#pragma once

#include <wx/frame.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include "RunThread.h"

class TerminalControl;
class DirectoryTreeControl;
class SourceFileNotebookControl;

class EditorFrame : public wxFrame
{
public:
	EditorFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size);
	virtual ~EditorFrame();

	enum
	{
		ID_Exit,
		ID_About,
		ID_Save,
		ID_Open,
		ID_Close,
		ID_SaveAll,
		ID_CloseAll,
		ID_OpenDirectory,
		ID_CloseDirectory,
		ID_RunWithDebugger,
		ID_RunWithoutDebugger,
		ID_KillScript
	};

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnSaveFile(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnCloseFile(wxCommandEvent& event);
	void OnSaveAllFiles(wxCommandEvent& event);
	void OnCloseAllFiles(wxCommandEvent& event);
	void OnOpenDirectory(wxCommandEvent& event);
	void OnCloseDirectory(wxCommandEvent& event);
	void OnRunWithDebugger(wxCommandEvent& event);
	void OnRunWithoutDebugger(wxCommandEvent& event);
	void OnKillScript(wxCommandEvent& event);
	void OnUpdateMenuItemUI(wxUpdateUIEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnRunThreadEntering(wxThreadEvent& event);
	void OnRunThreadExiting(wxThreadEvent& event);
	void OnRunThreadException(RunThreadExceptionEvent& event);
	void OnRunThreadOutput(RunThreadOutputEvent& event);
	void OnRunThreadInput(RunThreadInputEvent& event);
	void OnTerminalInputReady(wxCommandEvent& event);

	void UpdateTreeControl(void);
	void SaveWindowAdjustments();
	void RestoreWindowAdjustments();

	wxSplitterWindow* verticalSplitter;
	wxSplitterWindow* horizontalSplitter;
	DirectoryTreeControl* directoryTreeControl;
	SourceFileNotebookControl* sourceFileNotebookControl;
	TerminalControl* terminalControl;
};