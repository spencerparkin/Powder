#pragma once

#include "Panel.h"
#include "Scope.h"
#include "Value.h"
#include <wx/treectrl.h>

class ValuesPanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(ValuesPanel);

	ValuesPanel();
	virtual ~ValuesPanel();

	virtual bool GetPaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual bool MakeControls(void) override;
	virtual void OnNotified(Notification notification, void* notifyData) override;

	enum
	{
		ID_ModifyValue = wxID_HIGHEST + 2000,
		ID_ItemCollapsed,
		ID_ItemExpanded
	};

	void OnContextMenu(wxTreeEvent& event);
	void OnContextMenu_ModifyValue(wxCommandEvent& event);
	void OnUpdateMenuItemUI(wxUpdateUIEvent& event);
	void OnTreeItemCollapsed(wxTreeEvent& event);
	void OnTreeItemExpanded(wxTreeEvent& event);
	void OnTreeItemDoubleClicked(wxTreeEvent& event);

	void RebuildValueTree(void);

	class ScopeTreeItemData : public wxTreeItemData
	{
	public:
		ScopeTreeItemData(Powder::Scope* scope)
		{
			this->scope = scope;
		}

		virtual ~ScopeTreeItemData()
		{
		}

		Powder::Scope* scope;
	};

	class ValueTreeItemData : public wxTreeItemData
	{
	public:
		ValueTreeItemData(const wxString& name, Powder::Value* value)
		{
			this->name = name;
			this->value = value;
		}

		virtual ~ValueTreeItemData()
		{
		}

		wxString CalcLabel()
		{
			std::string valueStr = value->ToString();
			return wxString::Format("%s (0x%p): %s", (const char*)this->name.c_str(), (void*)value, valueStr.c_str());
		}

		wxString name;
		Powder::Value* value;
	};

	wxTreeItemId GenerateScopeItems(Powder::Scope* scope);
	void GenerateValueItems(wxTreeItemId parentItemId);
	void GenerateTreeForValue(wxTreeItemId parentItemId, const wxString& name, Powder::Value* value, std::set<Powder::Value*>& valueSet);
	void ApplyExpansionMap(wxTreeItemId parentItemId);

	wxTreeCtrl* valueTreeControl;
	wxTreeItemData* contextMenuItemData;
	wxTreeItemId contextMenuItemId;
	std::set<std::string> expansionMap;
};