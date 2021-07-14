#include "ValuesPanel.h"
#include "RunThread.h"

wxIMPLEMENT_DYNAMIC_CLASS(ValuesPanel, Panel);

ValuesPanel::ValuesPanel()
{
}

/*virtual*/ ValuesPanel::~ValuesPanel()
{
}

/*virtual*/ bool ValuesPanel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Caption("Values");
	paneInfo.Dockable();
	paneInfo.BestSize(600, 300);
	paneInfo.CloseButton(false);
	paneInfo.Name("Values");
	return true;
}

/*virtual*/ bool ValuesPanel::MakeControls(void)
{
	
	return true;
}

/*virtual*/ void ValuesPanel::OnNotified(Notification notification, void* notifyData)
{
	if (notification == RUNTHREAD_SUSPENDED)
	{
		// TODO: It should be safe to access the VM to read the values.  Note that I'm going
		//       to just update all values here.  I suppose this could be a problem if there
		//       were a lot of values, or values with a lot of contents, but it will work for
		//       now.  We want to always accurately reflect what's in the VM.
	}
}