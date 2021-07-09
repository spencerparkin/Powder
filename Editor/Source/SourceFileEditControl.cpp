#include "SourceFileEditControl.h"
#include "SourceFileNotebookControl.h"
#include <wx/file.h>

SourceFileEditControl::SourceFileEditControl(wxWindow* parent, const wxString& filePath) : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0L)
{
	this->filePath = filePath;
	this->modified = false;
	this->ready = false;

	wxFont font;
	font.SetFaceName("Courier New");
	font.SetFamily(wxFONTFAMILY_MODERN);
	this->StyleSetFont(0, font);

	this->Bind(wxEVT_STC_CHANGE, &SourceFileEditControl::OnTextChanged, this, wxID_ANY);
}

/*virtual*/ SourceFileEditControl::~SourceFileEditControl()
{
}

void SourceFileEditControl::OnTextChanged(wxStyledTextEvent& event)
{
	if (this->ready && !this->modified)
	{
		this->modified = true;
		SourceFileNotebookControl* notebookControl = (SourceFileNotebookControl*)this->GetParent();
		notebookControl->UpdateTabLabelFor(this);
	}
}

wxString SourceFileEditControl::GetTabLabel()
{
	wxString label = this->GetFilename();
	if (this->modified)
		label += "*";
	return label;
}

wxString SourceFileEditControl::GetFilename()
{
	return this->filePath.GetName() + "." + this->filePath.GetExt();
}

bool SourceFileEditControl::LoadFile(void)
{
	wxFile file(this->filePath.GetFullPath(), wxFile::OpenMode::read);
	wxString fileContents;
	if (!file.ReadAll(&fileContents))
		return false;
	this->SetText(fileContents);
	this->ready = true;
	return true;
}

bool SourceFileEditControl::SaveFile(void)
{
	wxFile file(this->filePath.GetFullPath(), wxFile::OpenMode::write);
	wxString fileContent = this->GetText();
	if (!file.Write(fileContent))
		return false;
	this->modified = false;
	return true;
}