#pragma once

#include "Panel.h"

class EvalStackPanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(EvalStackPanel);

	EvalStackPanel();
	virtual ~EvalStackPanel();

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual bool MakeControls(void) override;
	virtual void OnNotified(Notification notification, void* notifyData) override;
};