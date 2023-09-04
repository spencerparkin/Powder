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

/*virtual*/ void DirectoryTreePanel::OnNotified(Notification notification, void* notifyData)
{
	switch (notification)
	{
		case DIRECTORY_OPENED:
		case DIRECTORY_CHANGED:
		case APP_OPENING:
		{
			this->directoryTreeControl->rootPath.AssignDir(wxGetApp().GetProjectDirectory());
			this->directoryTreeControl->RebuildTree();
			break;
		}
		case DIRECTORY_CLOSED:
		{
			this->directoryTreeControl->rootPath = "";
			this->directoryTreeControl->RebuildTree();
			break;
		}
	}
}