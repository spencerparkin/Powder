#pragma once

#include <wx/frame.h>
#include <wx/notebook.h>

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
		ID_SaveAll,
		ID_CloseAll,
		ID_OpenDirectory,
		ID_CloseDirectory,
		ID_RunWithDebugger,
		ID_RunWithoutDebugger,
		ID_AttachToVM,
		ID_DetachFromVM,
		ID_StartDebugging,
		ID_StopDebugging,
		ID_PauseDebugging
	};

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnSaveAll(wxCommandEvent& event);
	void OnCloseAll(wxCommandEvent& event);
	void OnOpenDirectory(wxCommandEvent& event);
	void OnCloseDirectory(wxCommandEvent& event);
	void OnRunWithDebugger(wxCommandEvent& event);
	void OnRunWithoutDebugger(wxCommandEvent& event);
	void OnAttachToVM(wxCommandEvent& event);
	void OnDetachFromVM(wxCommandEvent& event);
	void OnUpdateMenuItemUI(wxUpdateUIEvent& event);
	void OnClose(wxCloseEvent& event);

	DirectoryTreeControl* directoryTreeControl;
	SourceFileNotebookControl* sourceFileNotebookControl;
};