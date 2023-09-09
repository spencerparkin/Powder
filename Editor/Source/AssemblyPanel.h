#pragma once

#include "Panel.h"
#include "AssemblyControl.h"

class AssemblyPanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(AssemblyPanel);

	AssemblyPanel();
	virtual ~AssemblyPanel();

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual bool MakeControls(void) override;
	virtual void OnNotified(Notification notification, void* notifyData) override;

	AssemblyControl* assemblyControl;
};