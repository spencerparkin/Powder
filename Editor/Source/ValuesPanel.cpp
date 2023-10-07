#include "ValuesPanel.h"
#include "Executable.h"
#include "RunThread.h"
#include "EditorApp.h"
#include "ListValue.h"
#include "MapValue.h"
#include "ClosureValue.h"
#include "EditorApp.h"
#include "EditorFrame.h"
#include <wx/textdlg.h>
#include <wx/msgdlg.h>

wxIMPLEMENT_DYNAMIC_CLASS(ValuesPanel, Panel);

ValuesPanel::ValuesPanel()
{
	this->valueTreeControl = nullptr;
	this->contextMenuItemData = nullptr;
}

/*virtual*/ ValuesPanel::~ValuesPanel()
{
}

/*virtual*/ bool ValuesPanel::GetPaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Caption("Values");
	paneInfo.Dockable();
	paneInfo.BestSize(600, 300);
	paneInfo.Name("Values");
	return true;
}

/*virtual*/ bool ValuesPanel::MakeControls(void)
{
	this->valueTreeControl = new wxTreeCtrl(this, wxID_ANY);

	this->valueTreeControl->Bind(wxEVT_TREE_ITEM_MENU, &ValuesPanel::OnContextMenu, this);
	this->valueTreeControl->Bind(wxEVT_TREE_ITEM_COLLAPSED, &ValuesPanel::OnTreeItemCollapsed, this);
	this->valueTreeControl->Bind(wxEVT_TREE_ITEM_EXPANDED, &ValuesPanel::OnTreeItemExpanded, this);
	this->valueTreeControl->Bind(wxEVT_MENU, &ValuesPanel::OnContextMenu_ModifyValue, this, ID_ModifyValue);
	this->valueTreeControl->Bind(wxEVT_UPDATE_UI, &ValuesPanel::OnUpdateMenuItemUI, this, ID_ModifyValue);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->valueTreeControl, 1, wxGROW | wxALL, 0);
	this->SetSizer(boxSizer);

	return true;
}

/*virtual*/ void ValuesPanel::OnNotified(Notification notification, void* notifyData)
{
	switch (notification)
	{
		case RUNTHREAD_SUSPENDED:
		{
			this->RebuildValueTree();
			break;
		}
		case RUNTHREAD_ENDED:
		{
			this->valueTreeControl->DeleteAllItems();
			break;
		}
	}
}

void ValuesPanel::OnContextMenu(wxTreeEvent& event)
{
	this->contextMenuItemId = event.GetItem();
	this->contextMenuItemData = this->valueTreeControl->GetItemData(event.GetItem());

	wxMenu contextMenu;

	contextMenu.Append(new wxMenuItem(&contextMenu, ID_ModifyValue, "Modify"));

	this->valueTreeControl->PopupMenu(&contextMenu);
}

void ValuesPanel::OnContextMenu_ModifyValue(wxCommandEvent& event)
{
	ValueTreeItemData* valueTreeItemData = dynamic_cast<ValueTreeItemData*>(this->contextMenuItemData);
	if (valueTreeItemData)
	{
		wxString currentValueStr = valueTreeItemData->value->ToString().c_str();
		wxTextEntryDialog dialog(wxGetApp().GetFrame(), "Please enter the new value", "Modify Value", currentValueStr);
		if (wxID_OK == dialog.ShowModal())
		{
			std::string valueStr = (const char*)dialog.GetValue().c_str();
			if (!valueTreeItemData->value->FromString(valueStr))
				::wxMessageBox("Failed to alter value.  The value class might not support assignment from string.", "Error", wxOK | wxICON_ERROR, wxGetApp().GetFrame());
			else
			{
				if (dynamic_cast<Powder::ListValue*>(valueTreeItemData->value) ||
					dynamic_cast<Powder::MapValue*>(valueTreeItemData->value) ||
					dynamic_cast<Powder::ClosureValue*>(valueTreeItemData->value))
				{
					this->RebuildValueTree();
				}
				else
				{
					this->valueTreeControl->SetItemText(this->contextMenuItemId, valueTreeItemData->CalcLabel());
				}
			}
		}
	}
}

void ValuesPanel::OnUpdateMenuItemUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_ModifyValue:
		{
			event.Enable(dynamic_cast<ValueTreeItemData*>(this->contextMenuItemData) ? true : false);
			break;
		}
	}
}

void ValuesPanel::OnTreeItemCollapsed(wxTreeEvent& event)
{
	wxTreeItemId treeItemId = event.GetItem();
	std::string itemText = (const char*)this->valueTreeControl->GetItemText(treeItemId).c_str();
	this->expansionMap.erase(itemText);
}

void ValuesPanel::OnTreeItemExpanded(wxTreeEvent& event)
{
	wxTreeItemId treeItemId = event.GetItem();
	std::string itemText = (const char*)this->valueTreeControl->GetItemText(treeItemId).c_str();
	this->expansionMap.insert(itemText);
}

void ValuesPanel::RebuildValueTree(void)
{
	RunThread* runThread = wxGetApp().GetRunThread();
	if (runThread)
	{
		this->valueTreeControl->DeleteAllItems();

		return;		// TODO: Disable this for now until I can find a better solution.  No need for a total re-write, but just build the tree only so far as the expansion map?

		std::vector<Powder::Scope*> scopeArray;
		runThread->vm->GetAllCurrentScopes(scopeArray);

		for (int i = 0; i < (signed)scopeArray.size(); i++)
			this->GenerateScopeItems(scopeArray[i]);

		wxTreeItemId rootItemId = this->valueTreeControl->GetRootItem();
		if (rootItemId.IsOk())
		{
			this->GenerateValueItems(rootItemId);
			this->ApplyExpansionMap(rootItemId);
		}
	}
}

void ValuesPanel::ApplyExpansionMap(wxTreeItemId parentItemId)
{
	std::string itemText = (const char*)this->valueTreeControl->GetItemText(parentItemId);
	if (this->expansionMap.find(itemText) != this->expansionMap.end())
		this->valueTreeControl->Expand(parentItemId);

	wxTreeItemIdValue cookie;
	wxTreeItemId childItemId = this->valueTreeControl->GetFirstChild(parentItemId, cookie);
	while (childItemId.IsOk())
	{
		this->ApplyExpansionMap(childItemId);
		childItemId = this->valueTreeControl->GetNextChild(childItemId, cookie);
	}
}

wxTreeItemId ValuesPanel::GenerateScopeItems(Powder::Scope* scope)
{
	wxTreeItemId treeItemId;
	wxTreeItemId parentTreeItemId;

	Powder::Scope* containingScope = scope->GetContainingScope();
	if (containingScope)
		parentTreeItemId = this->GenerateScopeItems(containingScope);

	wxTreeItemId childItemId;
	if (parentTreeItemId.IsOk())
	{
		wxTreeItemIdValue cookie;
		childItemId = this->valueTreeControl->GetFirstChild(parentTreeItemId, cookie);
		while (childItemId.IsOk())
		{
			ScopeTreeItemData* scopeItemData = (ScopeTreeItemData*)this->valueTreeControl->GetItemData(childItemId);
			if (scopeItemData->scope == scope)
			{
				treeItemId = childItemId;
				break;
			}
			childItemId = this->valueTreeControl->GetNextChild(childItemId, cookie);
		}
	}

	if (!childItemId.IsOk())
	{
		wxString label = wxString::Format("Scope (0x%08x)", int(scope));
		if (parentTreeItemId.IsOk())
			treeItemId = this->valueTreeControl->AppendItem(parentTreeItemId, label, -1, -1, new ScopeTreeItemData(scope));
		else
		{
			wxTreeItemId rootItemId = this->valueTreeControl->GetRootItem();
			if (!rootItemId.IsOk())
				treeItemId = this->valueTreeControl->AddRoot(label, -1, -1, new ScopeTreeItemData(scope));
			else
				treeItemId = rootItemId;
		}
	}

	return treeItemId;
}

void ValuesPanel::GenerateValueItems(wxTreeItemId parentItemId)
{
	// TODO: This just bogs down for programs with tons of data, and it is not unreasonable at all for
	//       a program to have a ton of data, so really a better solution is needed here.  I think that
	//       the tree should be created only so far as it is explored.

	wxTreeItemIdValue cookie;
	wxTreeItemId childItemId = this->valueTreeControl->GetFirstChild(parentItemId, cookie);
	while (childItemId.IsOk())
	{
		this->GenerateValueItems(childItemId);
		childItemId = this->valueTreeControl->GetNextChild(childItemId, cookie);
	}

	wxTreeItemData* treeItemData = this->valueTreeControl->GetItemData(parentItemId);
	ScopeTreeItemData* scopeItemData = dynamic_cast<ScopeTreeItemData*>(treeItemData);
	if (scopeItemData)
	{
		std::set<Powder::Value*> valueSet;
		scopeItemData->scope->GetValueMap()->ForAllEntries([this, &parentItemId, &valueSet](const char* key, GC::Reference<Powder::Value, false>& valueRef) -> bool {
			this->GenerateTreeForValue(parentItemId, wxString(key), valueRef.Get(), valueSet);
			return true;
		});

		this->valueTreeControl->SortChildren(parentItemId);
	}
}

void ValuesPanel::GenerateTreeForValue(wxTreeItemId parentItemId, const wxString& name, Powder::Value* value, std::set<Powder::Value*>& valueSet)
{
	ValueTreeItemData* valueTreeItemData = new ValueTreeItemData(name, value);
	wxTreeItemId childItemId = this->valueTreeControl->AppendItem(parentItemId, valueTreeItemData->CalcLabel(), -1, -1, valueTreeItemData);

	if (valueSet.find(value) != valueSet.end())
	{
		this->valueTreeControl->AppendItem(childItemId, "Recursive!");
		return;
	}

	valueSet.insert(value);

	Powder::ListValue* listValue = dynamic_cast<Powder::ListValue*>(value);
	if (listValue)
	{
		// TODO: Don't try to show a list with a billion items in it.
		for (int i = 0; i < (signed)listValue->Length(); i++)
		{
			Powder::Value* subValue = (*listValue)[i];
			this->GenerateTreeForValue(childItemId, wxString::Format("%d", i), subValue, valueSet);
		}
	}

	Powder::MapValue* mapValue = dynamic_cast<Powder::MapValue*>(value);
	if (mapValue)
	{
		// TODO: Don't try to show a map with a billion items in it.
		Powder::HashMap<GC::Reference<Powder::Value, false>>& valueMap = mapValue->GetValueMap();
		valueMap.ForAllEntries([this, &childItemId, &valueSet](const char* key, GC::Reference<Powder::Value, false>& subValueRef) -> bool {
			this->GenerateTreeForValue(childItemId, wxString(key), subValueRef.Get(), valueSet);
			return true;
		});
	}

	Powder::ClosureValue* closureValue = dynamic_cast<Powder::ClosureValue*>(value);
	if (closureValue)
	{
		closureValue->scopeRef.Get()->GetValueMap()->ForAllEntries([this, &childItemId, &valueSet](const char* key, GC::Reference<Powder::Value, false>& subValueRef) -> bool {
			this->GenerateTreeForValue(childItemId, wxString(key), subValueRef.Get(), valueSet);
			return true;
		});
	}

	valueSet.erase(value);
}