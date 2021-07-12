#include "SourceFileNotebookControl.h"
#include "SourceFileEditControl.h"
#include "EditorApp.h"
#include "EditorFrame.h"
#include <wx/msgdlg.h>
#include <wx/tokenzr.h>

SourceFileNotebookControl::SourceFileNotebookControl(wxWindow* parent) : wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_CLOSE_ON_ALL_TABS)
{
	this->Bind(wxEVT_AUINOTEBOOK_BUTTON, &SourceFileNotebookControl::OnPageCloseButtonPushed, this, wxID_ANY);
}

/*virtual*/ SourceFileNotebookControl::~SourceFileNotebookControl()
{
}

bool SourceFileNotebookControl::CanClosePage(int pageNumber)
{
	SourceFileEditControl* editControl = (SourceFileEditControl*)this->GetPage(pageNumber);
	if (!editControl->modified)
		return true;
	
	wxString fileName = editControl->GetFileName();
	wxString messageText = wxString::Format("The file %s is modified.  Save before close?", fileName.c_str());
	int result = ::wxMessageBox(messageText, wxT("Save changed?"), wxICON_QUESTION | wxYES_NO | wxCANCEL, wxGetApp().GetFrame());
	if (result == wxYES)
	{
		if (editControl->SaveFile())
			return true;
		return false;
	}
	else if (result == wxNO)
		return true;
	else if (result == wxCANCEL)
		return false;

	return true;
}

void SourceFileNotebookControl::OnPageCloseButtonPushed(wxAuiNotebookEvent& event)
{
	if (this->CanClosePage(event.GetSelection()))
		event.Skip();	// Pretend like we didn't handle the event and let the base class handle it as usual.
}

bool SourceFileNotebookControl::OpenSourceFile(const wxString& filePath)
{
	int pageNumber = -1;
	SourceFileEditControl* editControl = this->FindEditControl(filePath, &pageNumber);
	if (editControl)
	{
		if (this->GetSelection() != pageNumber)
			this->ChangeSelection(pageNumber);
	}
	else
	{
		editControl = new SourceFileEditControl(this, filePath);
		if (!editControl->LoadFile())
		{
			wxString fileName = editControl->GetFileName();
			wxString messageText = wxString::Format("The file %s could not be loaded from disk.", fileName.c_str());
			::wxMessageBox(messageText, "Load failed!", wxICON_ERROR | wxOK, wxGetApp().GetFrame());
			return false;
		}

		this->AddPage(editControl, editControl->GetTabLabel(), true);
	}

	return true;
}

bool SourceFileNotebookControl::CloseSourceFile(int pageNumber)
{
	if (!this->CanClosePage(pageNumber))
		return false;

	return this->RemovePage(pageNumber);
}

bool SourceFileNotebookControl::CloseAllFiles(void)
{
	while(this->GetPageCount() > 0)
		if (!this->CloseSourceFile(0))
			return false;

	return true;
}

void SourceFileNotebookControl::SaveSourceFile(int pageNumber)
{
	SourceFileEditControl* editControl = (SourceFileEditControl*)this->GetPage(pageNumber);
	if (editControl)
	{
		if (editControl->SaveFile())
			this->UpdateTabLabelFor(editControl);
		else
		{
			wxString fileName = editControl->GetFileName();
			wxString messageText = wxString::Format("The file %s could not be saved to disk.", fileName.c_str());
			::wxMessageBox(messageText, "Save failed!", wxICON_ERROR | wxOK, wxGetApp().GetFrame());
		}
	}
}

void SourceFileNotebookControl::SaveAllFiles(void)
{
	for (int i = 0; i < (signed)this->GetPageCount(); i++)
		this->SaveSourceFile(i);
}

void SourceFileNotebookControl::UpdateTabLabelFor(SourceFileEditControl* editControl)
{
	int pageNumber = -1;
	this->FindEditControl(editControl->filePath, &pageNumber);
	if (pageNumber >= 0)
		this->SetPageText(pageNumber, editControl->GetTabLabel());
}

SourceFileEditControl* SourceFileNotebookControl::FindEditControl(const wxFileName& filePath, int* pageNumber /*= nullptr*/)
{
	for (int i = 0; i < (signed)this->GetPageCount(); i++)
	{
		SourceFileEditControl* editControl = (SourceFileEditControl*)this->GetPage(i);
		if (editControl->filePath == filePath)
		{
			if (pageNumber)
				*pageNumber = i;

			return editControl;
		}
	}

	return nullptr;
}

SourceFileEditControl* SourceFileNotebookControl::GetSelectedEditControl()
{
	SourceFileEditControl* editControl = nullptr;
	int pageNumber = this->GetSelection();
	if(pageNumber >= 0)
		editControl = (SourceFileEditControl*)this->GetPage(pageNumber);
	return editControl;
}

bool SourceFileNotebookControl::AnyFilesModified()
{
	for (int i = 0; i < (signed)this->GetPageCount(); i++)
	{
		SourceFileEditControl* editControl = (SourceFileEditControl*)this->GetPage(i);
		if (editControl->modified)
			return true;
	}

	return false;
}

int SourceFileNotebookControl::OpenFileCount()
{
	return (signed)this->GetPageCount();
}

void SourceFileNotebookControl::RememberCurrentlyOpenFiles()
{
	wxString openFiles;
	for (int i = 0; i < (signed)this->GetPageCount(); i++)
	{
		SourceFileEditControl* editControl = (SourceFileEditControl*)this->GetPage(i);
		wxString fileName = editControl->filePath.GetFullPath();
		if (openFiles.length() > 0)
			openFiles += "|";
		openFiles += fileName;
	}

	wxGetApp().GetConfig()->Write("openFiles", openFiles);
}

void SourceFileNotebookControl::RestoreOpenFiles()
{
	wxString openFiles = wxGetApp().GetConfig()->Read("openFiles");
	if (!openFiles.empty())
	{
		wxStringTokenizer tokenizer(openFiles, "|");
		while (tokenizer.HasMoreTokens())
		{
			wxFileName fileName = tokenizer.GetNextToken();
			if (fileName.Exists())
				this->OpenSourceFile(fileName.GetFullPath());
		}
	}
}