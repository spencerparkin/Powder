#include "SourceFileNotebookControl.h"
#include "SourceFileEditControl.h"
#include "EditorApp.h"
#include "EditorFrame.h"
#include <wx/msgdlg.h>

SourceFileNotebookControl::SourceFileNotebookControl(wxWindow* parent) : wxNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxNB_MULTILINE)
{
}

/*virtual*/ SourceFileNotebookControl::~SourceFileNotebookControl()
{
}

void SourceFileNotebookControl::ToggleSourceFile(const wxString& filePath)
{
	if (!this->OpenSourceFile(filePath))
	{
		int pageNumber = -1;
		SourceFileEditControl* editControl = this->FindEditControl(filePath, &pageNumber);
		if (pageNumber >= 0)
			this->CloseSourceFile(pageNumber);
	}
}

bool SourceFileNotebookControl::OpenSourceFile(const wxString& filePath)
{
	int pageNumber = -1;
	SourceFileEditControl* editControl = this->FindEditControl(filePath, &pageNumber);
	if (editControl)
	{
		if (this->GetSelection() != pageNumber)
		{
			this->ChangeSelection(pageNumber);
			return true;
		}
	}
	else
	{
		editControl = new SourceFileEditControl(this, filePath);
		editControl->LoadFile();
		this->AddPage(editControl, editControl->GetTabLabel(), true);
		return true;
	}

	return false;
}

bool SourceFileNotebookControl::CloseSourceFile(int pageNumber)
{
	SourceFileEditControl* editControl = (SourceFileEditControl*)this->GetPage(pageNumber);
	if (editControl->modified)
	{
		wxString fileName = editControl->GetFilename();
		wxString messageText = wxString::Format("The file %s is modified.  Save before close?", fileName.c_str());
		int result = ::wxMessageBox(messageText, wxT("Save changed?"), wxICON_QUESTION | wxYES_NO | wxCANCEL, wxGetApp().frame);
		if (result == wxCANCEL)
			return false;
		else if (result == wxYES)
			editControl->SaveFile();
	}

	this->RemovePage(pageNumber);
	return true;
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
			wxString fileName = editControl->GetFilename();
			wxString messageText = wxString::Format("The file %s could not be saved to disk.", fileName.c_str());
			::wxMessageBox(messageText, "Save failed!", wxICON_ERROR | wxOK, wxGetApp().frame);
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