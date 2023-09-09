#pragma once

#include "Panel.h"
#include "Executor.h"
#include <wx/listctrl.h>

class EvalStackPanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(EvalStackPanel);

	EvalStackPanel();
	virtual ~EvalStackPanel();

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual bool MakeControls(void) override;
	virtual void OnNotified(Notification notification, void* notifyData) override;

	void RebuildStackList(Powder::Executor* executor);

	wxListCtrl* evalStackCtrl;
};