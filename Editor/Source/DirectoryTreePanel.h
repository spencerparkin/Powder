#pragma once

#include "Panel.h"

class DirectoryTreeControl;

class DirectoryTreePanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(DirectoryTreePanel);

	DirectoryTreePanel();
	virtual ~DirectoryTreePanel();

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual bool MakeControls(void) override;
	virtual void OnNotified(Notification notification) override;

	DirectoryTreeControl* directoryTreeControl;
};