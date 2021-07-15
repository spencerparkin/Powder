#include "TerminalPanel.h"
#include "TerminalControl.h"
#include "RunThread.h"
#include "EditorApp.h"

wxIMPLEMENT_DYNAMIC_CLASS(TerminalPanel, Panel);

TerminalPanel::TerminalPanel()
{
	this->terminalControl = nullptr;
}

/*virtual*/ TerminalPanel::~TerminalPanel()
{
}

/*virtual*/ bool TerminalPanel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Caption("Terminal");
	paneInfo.Dockable();
	paneInfo.BestSize(600, 300);
	paneInfo.CloseButton(false);
	paneInfo.Name("Terminal");
	return true;
}

/*virtual*/ bool TerminalPanel::MakeControls(void)
{
	this->terminalControl = new TerminalControl(this);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->terminalControl, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);

	return true;
}

/*virtual*/ void TerminalPanel::OnNotified(Notification notification, void* notifyData)
{
	switch (notification)
	{
		case RUNTHREAD_STARTED:
		{
			this->terminalControl->Clear();
			break;
		}
		case RUNTHREAD_ENDED:
		{
			RunThread* runThread = wxGetApp().GetRunThread();
			this->terminalControl->AppendText(wxString::Format("\n\nExecution time: %d ms", runThread->executionTimeMilliseconds));
			break;
		}
	}
}