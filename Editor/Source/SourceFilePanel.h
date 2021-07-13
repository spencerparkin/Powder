#pragma once

#include "Panel.h"

class SourceFileNotebookControl;

class SourceFilePanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(SourceFilePanel);

	SourceFilePanel();
	virtual ~SourceFilePanel();

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual bool MakeControls(void) override;
	virtual void OnNotified(Notification notification) override;

	SourceFileNotebookControl* notebookControl;
};