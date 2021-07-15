#include "Panel.h"

wxIMPLEMENT_DYNAMIC_CLASS(Panel, wxPanel);

Panel::Panel()
{
	this->menuItemId = -1;
}

/*virtual*/ Panel::~Panel()
{
}

/*virtual*/ bool Panel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	return false;
}

/*virtual*/ bool Panel::MakeControls(void)
{
	return false;
}

/*virtual*/ void Panel::OnNotified(Notification notification, void* notifyData)
{
}