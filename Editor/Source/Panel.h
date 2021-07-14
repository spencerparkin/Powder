#pragma once

#include <wx/panel.h>
#include <wx/aui/aui.h>

class Panel : public wxPanel
{
public:
	wxDECLARE_DYNAMIC_CLASS(Panel);

	Panel();
	virtual ~Panel();

	enum Notification
	{
		APP_OPENING,
		APP_CLOSING,
		DIRECTORY_OPENED,
		DIRECTORY_CLOSED,
		RUNTHREAD_STARTED,
		RUNTHREAD_ENDED,
		BREAKPOINTS_CHANGED
	};

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo);
	virtual bool MakeControls(void);
	virtual void OnNotified(Notification notification, void* notifyData);
};