#pragma once

#include <wx/frame.h>
#include <wx/aui/aui.h>
#include "RunThread.h"
#include "Panel.h"

class EditorFrame : public wxFrame
{
public:
	EditorFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size);
	virtual ~EditorFrame();

	enum
	{
		ID_Exit = wxID_HIGHEST,
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
		ID_StepOver,
		ID_StepInto,
		ID_StepOut,
		ID_PauseScript,
		ID_ResumeScript,
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
	void OnPauseScript(wxCommandEvent& event);
	void OnResumeScript(wxCommandEvent& event);
	void OnKillScript(wxCommandEvent& event);
	void OnUpdateMenuItemUI(wxUpdateUIEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnRunThreadEntering(wxThreadEvent& event);
	void OnRunThreadExiting(wxThreadEvent& event);
	void OnRunThreadException(RunThreadExceptionEvent& event);
	void OnRunThreadOutput(RunThreadOutputEvent& event);
	void OnRunThreadInput(RunThreadInputEvent& event);
	void OnRunThreadSuspended(RunThreadSuspendedEvent& event);
	void OnTerminalInputReady(wxCommandEvent& event);
	void OnStepOver(wxCommandEvent& event);
	void OnStepInto(wxCommandEvent& event);
	void OnStepOut(wxCommandEvent& event);

	void SaveWindowAdjustments();
	void RestoreWindowAdjustments();
	void KickoffRunThread(bool debuggingEnabled);
	void MakePanels(void);
	void NotifyPanels(Panel::Notification notification);

	template<typename T>
	T* FindPanel(const wxString& panelName)
	{
		wxAuiPaneInfoArray& paneInfoArray = auiManager->GetAllPanes();
		for (int i = 0; i < (signed)paneInfoArray.GetCount(); i++)
			if (paneInfoArray[i].name == panelName)
				return wxDynamicCast(paneInfoArray[i].window, T);
		return nullptr;
	}

	wxAuiManager* auiManager;
};