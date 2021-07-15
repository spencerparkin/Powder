#include "DirectoryTreeControl.h"
#include "EditorFrame.h"
#include "SourceFilePanel.h"
#include "SourceFileNotebookControl.h"
#include "EditorApp.h"
#include <wx/dir.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>

// TODO: Add drag/drop so you can move files and folders around?

DirectoryTreeControl::DirectoryTreeControl(wxWindow* parent) : wxTreeCtrl(parent, wxID_ANY)
{
	this->contextMenuItemData = nullptr;

	this->Bind(wxEVT_TREE_ITEM_ACTIVATED, &DirectoryTreeControl::OnItemActivated, this, wxID_ANY);
	this->Bind(wxEVT_TREE_ITEM_MENU, &DirectoryTreeControl::OnContextMenu, this, wxID_ANY);
	this->Bind(wxEVT_MENU, &DirectoryTreeControl::OnContextMenu_NewFile, this, ID_NewFile);
	this->Bind(wxEVT_MENU, &DirectoryTreeControl::OnContextMenu_NewFolder, this, ID_NewFolder);
	this->Bind(wxEVT_MENU, &DirectoryTreeControl::OnContextMenu_Delete, this, ID_Delete);
	this->Bind(wxEVT_UPDATE_UI, &DirectoryTreeControl::OnUpdateMenuItemUI, this, ID_NewFile);
	this->Bind(wxEVT_UPDATE_UI, &DirectoryTreeControl::OnUpdateMenuItemUI, this, ID_NewFolder);
	this->Bind(wxEVT_UPDATE_UI, &DirectoryTreeControl::OnUpdateMenuItemUI, this, ID_Delete);
}

/*virtual*/ DirectoryTreeControl::~DirectoryTreeControl()
{
}

void DirectoryTreeControl::OnContextMenu(wxTreeEvent& event)
{
	this->contextMenuItemData = (ItemData*)this->GetItemData(event.GetItem());

	wxMenu contextMenu;

	contextMenu.Append(new wxMenuItem(&contextMenu, ID_NewFile, "New File"));
	contextMenu.Append(new wxMenuItem(&contextMenu, ID_NewFolder, "New Folder"));
	contextMenu.AppendSeparator();
	contextMenu.Append(new wxMenuItem(&contextMenu, ID_Delete, "Delete"));

	this->PopupMenu(&contextMenu);
}

void DirectoryTreeControl::OnUpdateMenuItemUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_NewFile:
		case ID_NewFolder:
		{
			event.Enable(this->contextMenuItemData->path.IsDir());
			break;
		}
		case ID_Delete:
		{
			event.Enable(true);
			break;
		}
	}
}

void DirectoryTreeControl::OnContextMenu_NewFile(wxCommandEvent& event)
{
	wxTextEntryDialog dialog(wxGetApp().GetFrame(), "Please enter file name", "New File");
	if (wxID_OK == dialog.ShowModal())
	{
		wxFileName fileName = this->contextMenuItemData->path.GetFullPath() + "\\" + dialog.GetValue();
		fileName.SetExt("pow");
		wxFile file;
		if (!file.Open(fileName.GetFullPath(), wxFile::OpenMode::write))
			::wxMessageBox("Failed to create file: " + fileName.GetFullPath(), "Error", wxOK | wxICON_ERROR, wxGetApp().GetFrame());
		else
		{
			file.Write("# " + fileName.GetName() + "." + fileName.GetExt() + "\n\n# Your amazing code goes here...");
			file.Close();
			this->RebuildTree();
		}
	}
}

void DirectoryTreeControl::OnContextMenu_NewFolder(wxCommandEvent& event)
{
	wxTextEntryDialog dialog(wxGetApp().GetFrame(), "Please enter folder name", "New Folder");
	if (wxID_OK == dialog.ShowModal())
	{
		wxString folderName = dialog.GetValue();
		wxFileName folderPath = this->contextMenuItemData->path.GetFullPath() + wxString("\\") + wxString(folderName) + wxString("\\");
		if (!folderPath.Mkdir())
			::wxMessageBox("Failed to create directory: " + folderPath.GetFullPath(), "Error", wxOK | wxICON_ERROR, wxGetApp().GetFrame());
		else
			this->RebuildTree();
	}
}

void DirectoryTreeControl::OnContextMenu_Delete(wxCommandEvent& event)
{
	if (!this->contextMenuItemData->path.IsDir())
	{
		if (wxYES == ::wxMessageBox("Delete file?\n\n" + this->contextMenuItemData->path.GetFullPath(), "Delete", wxYES_NO | wxICON_QUESTION, wxGetApp().GetFrame()))
		{
			if (!::wxRemoveFile(this->contextMenuItemData->path.GetFullPath()))
				::wxMessageBox("Failed to delete file: " + this->contextMenuItemData->path.GetFullPath(), "Error", wxOK | wxICON_ERROR, wxGetApp().GetFrame());
			else
				this->RebuildTree();
		}
	}
	else
	{
		if (wxYES == ::wxMessageBox("Delete folder?\n\n" + this->contextMenuItemData->path.GetFullPath(), "Delete", wxYES_NO | wxICON_QUESTION, wxGetApp().GetFrame()))
		{
			if (!wxDir::Remove(this->contextMenuItemData->path.GetFullPath()))
				::wxMessageBox("Failed to delete directory: " + this->contextMenuItemData->path.GetFullPath(), "Error", wxOK | wxICON_ERROR, wxGetApp().GetFrame());
			else
				this->RebuildTree();
		}
	}
}

void DirectoryTreeControl::OnItemActivated(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	if (itemId.IsOk())
	{
		ItemData* itemData = dynamic_cast<ItemData*>(this->GetItemData(itemId));
		if (itemData->path.FileExists() && !itemData->path.IsDir())
		{
			SourceFilePanel* sourceFilePanel = wxGetApp().GetFrame()->FindPanel<SourceFilePanel>("SourceFile");
			if (sourceFilePanel)
				sourceFilePanel->notebookControl->OpenSourceFile(itemData->path.GetFullPath());
		}
	}
}

void DirectoryTreeControl::ParsePathParts(const wxString& fullPath, wxArrayString& pathPartsArray)
{
	pathPartsArray.Clear();
	wxStringTokenizer tokenizer(fullPath, "\\");
	while (tokenizer.HasMoreTokens())
		pathPartsArray.Add(tokenizer.GetNextToken());
}

void DirectoryTreeControl::RebuildTree(void)
{
	this->DeleteAllItems();
	
	if (this->rootPath.IsDir())
	{
		class DirectoryTraverser : public wxDirTraverser
		{
		public:
			DirectoryTraverser()
			{
			}

			virtual ~DirectoryTraverser()
			{
			}

			virtual wxDirTraverseResult OnFile(const wxString& filePath) override
			{
				this->pathArray.Add(filePath);
				return wxDIR_CONTINUE;
			}

			virtual wxDirTraverseResult OnDir(const wxString& dirPath) override
			{
				this->pathArray.Add(dirPath);
				return wxDIR_CONTINUE;
			}

			wxArrayString pathArray;
		};

		DirectoryTraverser dirTraversor;
		wxDir dir(this->rootPath.GetFullPath());
		dir.Traverse(dirTraversor, "*.pow", wxDIR_FILES | wxDIR_DIRS);

		wxArrayString rootPathPartsArray;
		this->ParsePathParts(this->rootPath.GetFullPath(), rootPathPartsArray);
		wxString rootDirName = rootPathPartsArray[rootPathPartsArray.GetCount() - 1];
		this->AddRoot(rootDirName, -1, -1, new ItemData(this->rootPath));

		for (int i = 0; i < (signed)dirTraversor.pathArray.Count(); i++)
		{
			wxString path = dirTraversor.pathArray[i];

			wxArrayString pathPartsArray;
			this->ParsePathParts(path, pathPartsArray);

			for (int j = 0; j < (signed)rootPathPartsArray.GetCount(); j++)
				pathPartsArray.RemoveAt(0);

			this->BuildPath(pathPartsArray);
		}

		this->ExpandAll();
	}
}

void DirectoryTreeControl::BuildPath(const wxArrayString& pathPartsArray)
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
		{
			wxString filePath = this->rootPath.GetFullPath();
			for (int j = 0; j < (signed)pathPartsArray.GetCount(); j++)
				filePath += "\\" + pathPartsArray[j];

			if (wxDirExists(filePath))
				filePath += "\\";
			
			ItemData* itemData = new ItemData(filePath);
			parentItemId = this->AppendItem(parentItemId, label, -1, -1, itemData);
		}
	}
}

DirectoryTreeControl::ItemData::ItemData(const wxFileName& path)
{
	this->path = path;
}

/*virtual*/ DirectoryTreeControl::ItemData::~ItemData()
{
}