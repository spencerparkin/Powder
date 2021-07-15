#include "ValuesPanel.h"
#include "RunThread.h"
#include "EditorApp.h"
#include "ListValue.h"
#include "MapValue.h"
#include "ClosureValue.h"

wxIMPLEMENT_DYNAMIC_CLASS(ValuesPanel, Panel);

ValuesPanel::ValuesPanel()
{
	this->valueTreeControl = nullptr;
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

void ValuesPanel::RebuildValueTree(void)
{
	RunThread* runThread = wxGetApp().GetRunThread();
	if (runThread)
	{
		this->valueTreeControl->DeleteAllItems();

		std::vector<Powder::Scope*> scopeArray;
		runThread->vm->GetAllCurrentScopes(scopeArray);

		for (int i = 0; i < (signed)scopeArray.size(); i++)
			this->GenerateScopeItems(scopeArray[i]);

		wxTreeItemId rootItemId = this->valueTreeControl->GetRootItem();
		if (rootItemId.IsOk())
			this->GenerateValueItems(rootItemId);

		this->valueTreeControl->ExpandAll();
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
		scopeItemData->scope->GetValueMap()->ForAllEntries([this, &parentItemId](const char* key, Powder::Value* value) -> bool {
			this->GenerateTreeForValue(parentItemId, wxString(key), value);
			return true;
		});
	}
}

void ValuesPanel::GenerateTreeForValue(wxTreeItemId parentItemId, const wxString& name, Powder::Value* value)
{
	std::string valueStr = value->ToString();
	wxTreeItemId childItemId = this->valueTreeControl->AppendItem(parentItemId, wxString::Format("%s (0x%08x): %s", name, int(value), valueStr.c_str()), -1, -1, new ValueTreeItemData(value));

	Powder::ListValue* listValue = dynamic_cast<Powder::ListValue*>(value);
	if (listValue)
	{
		for (int i = 0; i < (signed)listValue->Length(); i++)
		{
			Powder::Value* subValue = (*listValue)[i];
			this->GenerateTreeForValue(childItemId, wxString::Format("%d", i), subValue);
		}
	}

	Powder::MapValue* mapValue = dynamic_cast<Powder::MapValue*>(value);
	if (mapValue)
	{
		Powder::HashMap<Powder::Value*>& valueMap = mapValue->GetValueMap();
		valueMap.ForAllEntries([this, &childItemId](const char* key, Powder::Value* subValue) -> bool {
			this->GenerateTreeForValue(childItemId, wxString(key), subValue);
			return true;
		});
	}

	Powder::ClosureValue* closureValue = dynamic_cast<Powder::ClosureValue*>(value);
	if (closureValue)
	{
		closureValue->scope.Get()->GetValueMap()->ForAllEntries([this, &childItemId](const char* key, Powder::Value* subValue) -> bool {
			this->GenerateTreeForValue(childItemId, wxString(key), subValue);
			return true;
		});
	}
}