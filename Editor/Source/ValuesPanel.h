#pragma once

#include "Panel.h"

class ValuesPanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(ValuesPanel);

	ValuesPanel();
	virtual ~ValuesPanel();

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual bool MakeControls(void) override;
	virtual void OnNotified(Notification notification, void* notifyData) override;
};