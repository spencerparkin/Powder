#pragma once

#include <wx/treectrl.h>
#include <wx/filename.h>

class DirectoryTreeControl : public wxTreeCtrl
{
public:
	DirectoryTreeControl(wxWindow* parent);
	virtual ~DirectoryTreeControl();

	void RebuildTree(void);

	wxFileName rootPath;

private:
	
	enum
	{
		ID_NewFile = wxID_HIGHEST,
		ID_NewFolder,
		ID_Delete
	};

	void OnItemActivated(wxTreeEvent& event);
	void OnContextMenu(wxTreeEvent& event);
	void OnContextMenu_NewFile(wxCommandEvent& event);
	void OnContextMenu_NewFolder(wxCommandEvent& event);
	void OnContextMenu_Delete(wxCommandEvent& event);
	void OnUpdateMenuItemUI(wxUpdateUIEvent& event);

	class ItemData : public wxTreeItemData
	{
	public:
		ItemData(const wxFileName& path);
		virtual ~ItemData();

		wxFileName path;
	};

	void BuildPath(const wxArrayString& pathPartsArray);
	void ParsePathParts(const wxString& fullPath, wxArrayString& pathPartsArray);

	ItemData* contextMenuItemData;
};