#include "DirectoryTreeControl.h"
#include "EditorFrame.h"
#include "SourceFileNotebookControl.h"
#include "EditorApp.h"
#include <wx/dir.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <wx/filename.h>

DirectoryTreeControl::DirectoryTreeControl(wxWindow* parent) : wxTreeCtrl(parent, wxID_ANY)
{
	this->Bind(wxEVT_TREE_ITEM_ACTIVATED, &DirectoryTreeControl::OnItemActivated, this, wxID_ANY);
}

/*virtual*/ DirectoryTreeControl::~DirectoryTreeControl()
{
}

void DirectoryTreeControl::OnItemActivated(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	if (itemId.IsOk())
	{
		ItemData* itemData = dynamic_cast<ItemData*>(this->GetItemData(itemId));
		wxGetApp().frame->sourceFileNotebookControl->OpenSourceFile(itemData->filePath);
	}
}

void DirectoryTreeControl::RebuildForDirectory(const wxString& directoryPath)
{
	wxString containingFolder, folderName;
	wxFileName::SplitPath(directoryPath, &containingFolder, &folderName, nullptr);

	this->DeleteAllItems();
	this->AddRoot(folderName);

	wxArrayString fileArray;
	wxDir::GetAllFiles(directoryPath, &fileArray, wxEmptyString, wxDIR_DIRS | wxDIR_FILES);

	for (int i = 0; i < (signed)fileArray.Count(); i++)
	{
		wxFileName filePath = fileArray[i];
		if (filePath.GetExt() != "pow")
			continue;

		wxStringTokenizer tokenizer(filePath.GetFullPath(), "\\");
		wxArrayString pathPartsArray;
		while (tokenizer.HasMoreTokens())
			pathPartsArray.Add(tokenizer.GetNextToken());

		while (true)
		{
			wxString part = pathPartsArray[0];
			if (part != folderName)
				pathPartsArray.RemoveAt(0);
			else
				break;
		}

		pathPartsArray.RemoveAt(0);
		this->BuildPath(pathPartsArray, new ItemData(filePath.GetFullPath()));
	}

	this->ExpandAll();
}

void DirectoryTreeControl::BuildPath(const wxArrayString& pathPartsArray, ItemData* itemData)
{
	wxTreeItemId parentItemId = this->GetRootItem();

	for (int i = 0; i < (signed)pathPartsArray.Count(); i++)
	{
		wxString label = pathPartsArray[i];

		wxTreeItemIdValue cookie;
		wxTreeItemId childItemId = this->GetFirstChild(parentItemId, cookie);
		while (childItemId.IsOk())
		{
			if (this->GetItemText(childItemId) == label)
			{
				parentItemId = childItemId;
				break;
			}

			childItemId = this->GetNextChild(childItemId, cookie);
		}

		if (!childItemId.IsOk())
			parentItemId = this->AppendItem(parentItemId, label, -1, -1, (i == pathPartsArray.Count() - 1) ? itemData : nullptr);
	}
}

DirectoryTreeControl::ItemData::ItemData(const wxString& filePath)
{
	this->filePath = filePath;
}

/*virtual*/ DirectoryTreeControl::ItemData::~ItemData()
{
}