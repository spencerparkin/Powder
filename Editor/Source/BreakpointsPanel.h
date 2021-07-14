#pragma once

#include "Panel.h"
#include <wx/listctrl.h>
#include <list>

class BreakpointsPanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(BreakpointsPanel);

	BreakpointsPanel();
	virtual ~BreakpointsPanel();

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual bool MakeControls(void) override;
	virtual void OnNotified(Notification notification, void* notifyData) override;

	void OnDeleteAllBreakpoints(wxCommandEvent& event);
	void OnBreakpointsListItemActivated(wxListEvent& event);
	void OnKeyUp(wxKeyEvent& event);

	void RebuildBreakpointsList();

	wxListView* listViewControl;
};