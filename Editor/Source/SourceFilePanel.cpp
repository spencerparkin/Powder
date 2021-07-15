#include "SourceFilePanel.h"
#include "SourceFileNotebookControl.h"
#include "SourceFileEditControl.h"
#include "RunThread.h"

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
	switch (notification)
	{
		case APP_OPENING:
		{
			this->notebookControl->RestoreOpenFiles();
			break;
		}
		case APP_CLOSING:
		{
			this->notebookControl->RememberCurrentlyOpenFiles();
			break;
		}
		case RUNTHREAD_ENDED:
		{
			this->notebookControl->ClearExecutionMarkers();
			break;
		}
		case BREAKPOINTS_CHANGED:
		{
			wxFileName* sourceFile = (wxFileName*)notifyData;
			if (sourceFile)
			{
				SourceFileEditControl* editControl = this->notebookControl->FindEditControl(*sourceFile);
				if (editControl)
					editControl->UpdateBreakpointMarkers();
			}
			else
			{
				for (int i = 0; i < (signed)this->notebookControl->GetPageCount(); i++)
				{
					SourceFileEditControl* editControl = (SourceFileEditControl*)this->notebookControl->GetPage(i);
					editControl->UpdateBreakpointMarkers();
				}
			}

			break;
		}
		case RUNTHREAD_SUSPENDED:
		{
			RunThreadSuspendedEvent* event = (RunThreadSuspendedEvent*)notifyData;
			if (this->notebookControl->OpenSourceFile(event->sourceFile))
			{
				SourceFileEditControl* editControl = this->notebookControl->FindEditControl(event->sourceFile);
				if (editControl)
					editControl->ShowExecutionSuspendedAt(event->lineNumber, event->columnNumber);
			}

			break;
		}
	}
}