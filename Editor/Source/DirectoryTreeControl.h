#pragma once

#include <wx/treectrl.h>

class DirectoryTreeControl : public wxTreeCtrl
{
public:
	DirectoryTreeControl(wxWindow* parent);
	virtual ~DirectoryTreeControl();

	void RebuildForDirectory(const wxString& directoryPath);

private:
	
	void OnItemActivated(wxTreeEvent& event);

	class ItemData : public wxTreeItemData
	{
	public:
		ItemData(const wxString& filePath);
		virtual ~ItemData();

		wxString filePath;
	};

	void BuildPath(const wxArrayString& pathPartsArray, ItemData* itemData);
};