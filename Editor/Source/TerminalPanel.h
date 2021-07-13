#pragma once

#include "Panel.h"

class TerminalControl;

class TerminalPanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(TerminalPanel);

	TerminalPanel();
	virtual ~TerminalPanel();

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual bool MakeControls(void) override;
	virtual void OnNotified(Notification notification) override;

	TerminalControl* terminalControl;
};