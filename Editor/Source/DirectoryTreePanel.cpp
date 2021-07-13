#include "DirectoryTreePanel.h"
#include "DirectoryTreeControl.h"
#include "EditorApp.h"

wxIMPLEMENT_DYNAMIC_CLASS(DirectoryTreePanel, Panel);

DirectoryTreePanel::DirectoryTreePanel()
{
	this->directoryTreeControl = nullptr;
}

/*virtual*/ DirectoryTreePanel::~DirectoryTreePanel()
{
}

/*virtual*/ bool DirectoryTreePanel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Caption("Directory Tree");
	paneInfo.Dockable();
	paneInfo.BestSize(600, 300);
	paneInfo.CloseButton(false);
	paneInfo.Name("DirectoryTree");
	return true;
}

/*virtual*/ bool DirectoryTreePanel::MakeControls(void)
{
	this->directoryTreeControl = new DirectoryTreeControl(this);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->directoryTreeControl, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);

	return true;
}

/*virtual*/ void DirectoryTreePanel::OnNotified(Notification notification)
{
	if (notification == DIRECTORY_OPENED || notification == APP_OPENING)
	{
		wxString directoryPath = wxGetApp().GetProjectDirectory();
		this->directoryTreeControl->RebuildForDirectory(directoryPath);
	}
	else if (notification == DIRECTORY_CLOSED)
	{
		this->directoryTreeControl->RebuildForDirectory("");
	}
}