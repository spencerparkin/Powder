#include "AssemblyPanel.h"
#include "RunThread.h"

wxIMPLEMENT_DYNAMIC_CLASS(AssemblyPanel, Panel);

AssemblyPanel::AssemblyPanel()
{
	this->assemblyControl = nullptr;
}

/*virtual*/ AssemblyPanel::~AssemblyPanel()
{
}

/*virtual*/ bool AssemblyPanel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Caption("Assembly");
	paneInfo.Dockable();
	paneInfo.BestSize(300, 300);
	paneInfo.Name("Assembly");
	paneInfo.Dock().Right();
	return true;
}

/*virtual*/ bool AssemblyPanel::MakeControls(void)
{
	this->assemblyControl = new AssemblyControl(this);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->assemblyControl, 1, wxGROW | wxALL, 0);
	this->SetSizer(boxSizer);

	return true;
}

/*virtual*/ void AssemblyPanel::OnNotified(Notification notification, void* notifyData)
{
	switch (notification)
	{
		case RUNTHREAD_STARTED:
		{
			this->assemblyControl->Clear();
			break;
		}
		case RUNTHREAD_SUSPENDED:
		{
			RunThreadSuspendedEvent* event = (RunThreadSuspendedEvent*)notifyData;
			this->assemblyControl->OnExecutionSuspended(event->programBufferLocation, event->instructionMapValue);
			break;
		}
	}
}