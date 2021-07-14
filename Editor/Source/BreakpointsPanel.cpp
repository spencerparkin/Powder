#include "BreakpointsPanel.h"
#include "EditorApp.h"
#include "EditorFrame.h"
#include "SourceFilePanel.h"
#include "SourceFileNotebookControl.h"
#include "SourceFileEditControl.h"
#include <wx/button.h>

wxIMPLEMENT_DYNAMIC_CLASS(BreakpointsPanel, Panel);

BreakpointsPanel::BreakpointsPanel()
{
	this->listViewControl = nullptr;
}

/*virtual*/ BreakpointsPanel::~BreakpointsPanel()
{
}

/*virtual*/ bool BreakpointsPanel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Caption("Breakpoints");
	paneInfo.Dockable();
	paneInfo.BestSize(300, 300);
	paneInfo.Name("Breakpoints");
	paneInfo.CloseButton(false);
	paneInfo.Dock().Right();
	return true;
}

/*virtual*/ bool BreakpointsPanel::MakeControls(void)
{
	this->listViewControl = new wxListView(this);
	this->listViewControl->AppendColumn("Source File");
	this->listViewControl->AppendColumn("Line Number");

	wxButton* deleteAllButton = new wxButton(this, wxID_ANY, "Delete All");

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->listViewControl, 1, wxGROW | wxALL, 0);
	boxSizer->Add(deleteAllButton, 0, wxGROW | wxALL, 0);
	this->SetSizer(boxSizer);

	this->Bind(wxEVT_BUTTON, &BreakpointsPanel::OnDeleteAllBreakpoints, this);
	this->Bind(wxEVT_LIST_ITEM_ACTIVATED, &BreakpointsPanel::OnBreakpointsListItemActivated, this);
	this->listViewControl->Bind(wxEVT_KEY_UP, &BreakpointsPanel::OnKeyUp, this);

	return true;
}

/*virtual*/ void BreakpointsPanel::OnNotified(Notification notification, void* notifyData)
{
	if (notification == BREAKPOINTS_CHANGED)
	{
		this->RebuildBreakpointsList();
	}
}

void BreakpointsPanel::OnKeyUp(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE)
	{
		wxCriticalSectionLocker locker(wxGetApp().breakpointListCS);
		std::set<std::string> sourceFileSet;
		int i = (int)this->listViewControl->GetFirstSelected();
		while (i >= 0)
		{
			EditorApp::Breakpoint* breakpoint = (EditorApp::Breakpoint*)this->listViewControl->GetItemData(i);
			wxString fullPath = breakpoint->sourceFile.GetFullPath();
			wxGetApp().ToggleBreakpoint(breakpoint->sourceFile, breakpoint->lineNumber, false);
			sourceFileSet.insert((const char*)fullPath.c_str());
			i = (int)this->listViewControl->GetNextSelected(i);
		}
		for (auto sourceFile : sourceFileSet)
		{
			wxFileName fileName(sourceFile.c_str());
			wxGetApp().GetFrame()->NotifyPanels(Panel::BREAKPOINTS_CHANGED, &fileName);
		}
		return;
	}

	event.Skip();
}

void BreakpointsPanel::RebuildBreakpointsList()
{
	this->listViewControl->DeleteAllItems();
	wxCriticalSectionLocker locker(wxGetApp().breakpointListCS);
	int i = 0;
	for (std::list<EditorApp::Breakpoint>::iterator iter = wxGetApp().breakpointList.begin(); iter != wxGetApp().breakpointList.end(); iter++)
	{
		EditorApp::Breakpoint& breakpoint = *iter;
		wxString sourceFile = breakpoint.sourceFile.GetName() + "." + breakpoint.sourceFile.GetExt();
		this->listViewControl->InsertItem(i, sourceFile);
		this->listViewControl->SetItem(i, 1, wxString::Format("%d", breakpoint.lineNumber));
		this->listViewControl->SetItemPtrData(i, (wxUIntPtr)&breakpoint);
		i++;
	}
}

void BreakpointsPanel::OnDeleteAllBreakpoints(wxCommandEvent& event)
{
	wxCriticalSectionLocker locker(wxGetApp().breakpointListCS);
	wxGetApp().breakpointList.clear();
	wxGetApp().GetFrame()->NotifyPanels(Panel::BREAKPOINTS_CHANGED, nullptr);
}

void BreakpointsPanel::OnBreakpointsListItemActivated(wxListEvent& event)
{
	SourceFilePanel* sourceFilePanel = wxGetApp().GetFrame()->FindPanel<SourceFilePanel>("SourceFile");
	if (sourceFilePanel)
	{
		EditorApp::Breakpoint* breakpoint = (EditorApp::Breakpoint*)event.GetItem().GetData();
		if (sourceFilePanel->notebookControl->OpenSourceFile(breakpoint->sourceFile.GetFullPath()))
		{
			SourceFileEditControl* editControl = sourceFilePanel->notebookControl->FindEditControl(breakpoint->sourceFile.GetFullPath());
			if (editControl)
			{
				editControl->ShowLineAndColumn(breakpoint->lineNumber, 1);
			}
		}
	}
}