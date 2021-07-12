#include "SourceFileEditControl.h"
#include "SourceFileNotebookControl.h"
#include "EditorFrame.h"
#include "EditorApp.h"
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

	this->SetMarginCount(2);
	this->SetMarginType(0, wxSTC_MARGIN_SYMBOL);
	this->SetMarginType(1, wxSTC_MARGIN_NUMBER);
	this->SetMarginWidth(0, 10);
	this->SetMarginWidth(1, 30);
	this->SetMarginSensitive(0, true);
	this->MarkerDefine(0, wxSTC_MARK_CIRCLE, *wxRED, *wxRED);

	this->Bind(wxEVT_STC_MODIFIED, &SourceFileEditControl::OnModified, this, wxID_ANY);
	//this->Bind(wxEVT_STC_KEY, &SourceFileEditControl::OnKeyPressed, this, wxID_ANY);
	this->Bind(wxEVT_STC_MARGINCLICK, &SourceFileEditControl::OnMarginClicked, this, wxID_ANY);
}

/*virtual*/ SourceFileEditControl::~SourceFileEditControl()
{
}

void SourceFileEditControl::OnMarginClicked(wxStyledTextEvent& event)
{
	if (event.GetMargin() == 0)
	{
		int line = this->LineFromPosition(event.GetPosition());
		int flags = this->MarkerGet(line);
		if ((flags & 0x1) != 0)
			this->MarkerDelete(line, 0);
		else
			this->MarkerAdd(line, 0);
	}
}

void SourceFileEditControl::OnKeyPressed(wxStyledTextEvent& event)
{
	wxString cursorLocationText = wxString::Format("Line: %d, Column: %d", this->GetCurrentLine(), this->GetCurrentPos());
	wxGetApp().GetFrame()->GetStatusBar()->SetStatusText(cursorLocationText, 1);
}

void SourceFileEditControl::OnModified(wxStyledTextEvent& event)
{
	if ((event.GetModificationType() & (wxSTC_MOD_INSERTTEXT | wxSTC_MOD_DELETETEXT)) != 0)
	{
		if (this->ready && !this->modified)
		{
			this->modified = true;
			SourceFileNotebookControl* notebookControl = (SourceFileNotebookControl*)this->GetParent();
			notebookControl->UpdateTabLabelFor(this);
		}
	}
}

wxString SourceFileEditControl::GetTabLabel()
{
	wxString label = this->GetFileName();
	if (this->modified)
		label += "*";
	return label;
}

wxString SourceFileEditControl::GetFileName()
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