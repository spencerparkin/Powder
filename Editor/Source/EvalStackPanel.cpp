#include "EvalStackPanel.h"

wxIMPLEMENT_DYNAMIC_CLASS(EvalStackPanel, Panel);

EvalStackPanel::EvalStackPanel()
{
}

/*virtual*/ EvalStackPanel::~EvalStackPanel()
{
}

/*virtual*/ bool EvalStackPanel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	return false;
}

/*virtual*/ bool EvalStackPanel::MakeControls(void)
{
	return false;
}

/*virtual*/ void EvalStackPanel::OnNotified(Notification notification, void* notifyData)
{
}