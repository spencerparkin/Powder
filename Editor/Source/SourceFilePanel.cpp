#include "SourceFilePanel.h"
#include "SourceFileNotebookControl.h"
#include "SourceFileEditControl.h"

wxIMPLEMENT_DYNAMIC_CLASS(SourceFilePanel, Panel);

SourceFilePanel::SourceFilePanel()
{
	this->notebookControl = nullptr;
}

/*virtual*/ SourceFilePanel::~SourceFilePanel()
{
}

/*virtual*/ bool SourceFilePanel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Caption("Source Files");
	paneInfo.Dockable();
	paneInfo.BestSize(600, 300);
	paneInfo.CloseButton(false);
	paneInfo.Name("SourceFile");
	paneInfo.CenterPane();
	return true;
}

/*virtual*/ bool SourceFilePanel::MakeControls(void)
{
	this->notebookControl = new SourceFileNotebookControl(this);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->notebookControl, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);

	return true;
}

/*virtual*/ void SourceFilePanel::OnNotified(Notification notification, void* notifyData)
{
	if (notification == APP_OPENING)
	{
		this->notebookControl->RestoreOpenFiles();
	}
	else if (notification == APP_CLOSING)
	{
		this->notebookControl->RememberCurrentlyOpenFiles();
	}
	else if (notification == RUNTHREAD_ENDED)
	{
		this->notebookControl->ClearExecutionMarkers();
	}
	else if (notification == BREAKPOINTS_CHANGED)
	{
		wxFileName* sourceFile = (wxFileName*)notifyData;
		SourceFileEditControl* editControl = this->notebookControl->FindEditControl(*sourceFile);
		if (editControl)
			editControl->UpdateBreakpointMarkers();
	}
}