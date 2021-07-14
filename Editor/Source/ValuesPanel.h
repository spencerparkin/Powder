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
		ValueTreeItemData(Powder::Value* value)
		{
			this->value = value;
		}

		virtual ~ValueTreeItemData()
		{
		}

		Powder::Value* value;
	};

	wxTreeItemId GenerateScopeItems(Powder::Scope* scope);
	void GenerateValueItems(wxTreeItemId treeItemId);

	wxTreeCtrl* valueTreeControl;
};