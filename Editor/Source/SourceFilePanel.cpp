#include "SourceFilePanel.h"
#include "SourceFileNotebookControl.h"
#include "SourceFileEditControl.h"
#include "RunThread.h"
#include "JsonValue.h"
#include "Executable.h"

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

			wxString sourceFile;
			const ParseParty::JsonString* sourceFileValue = dynamic_cast<const ParseParty::JsonString*>(event->executable->debugInfoDoc->GetValue("source_file"));
			if (sourceFileValue)
				sourceFile = sourceFileValue->GetValue();

			if (this->notebookControl->OpenSourceFile(sourceFile))
			{
				SourceFileEditControl* editControl = this->notebookControl->FindEditControl(sourceFile);
				if (editControl)
					editControl->ShowExecutionSuspendedAt(event->fileLocation.line, event->fileLocation.column);
			}

			break;
		}
	}
}