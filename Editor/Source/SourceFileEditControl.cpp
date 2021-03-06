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
	this->suspensionMarkerHandler = -1;

	wxFont font;
	font.SetFaceName("Courier New");
	font.SetFamily(wxFONTFAMILY_MODERN);
	this->StyleSetFont(0, font);

	this->SetMarginCount(2);
	this->SetMarginType(0, wxSTC_MARGIN_NUMBER);
	this->SetMarginWidth(0, 30);
	this->SetMarginSensitive(0, true);
	this->MarkerDefine(BREAKPOINT_MARKER, wxSTC_MARK_CIRCLE, *wxRED, *wxRED);
	this->MarkerDefine(EXECUTIONPOINT_MARKER, wxSTC_MARK_ARROW, *wxGREEN, *wxGREEN);

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
		int lineNumber = 1 + this->LineFromPosition(event.GetPosition());
		wxCriticalSectionLocker locker(wxGetApp().breakpointListCS);
		wxGetApp().ToggleBreakpoint(this->filePath, lineNumber);
	}
}

void SourceFileEditControl::UpdateBreakpointMarkers()
{
	this->MarkerDeleteAll(BREAKPOINT_MARKER);
	wxCriticalSectionLocker locker(wxGetApp().breakpointListCS);
	for (auto breakpoint : wxGetApp().breakpointList)
		if (breakpoint.sourceFile == this->filePath)
			this->MarkerAdd(breakpoint.lineNumber - 1, BREAKPOINT_MARKER);
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
	if (!this->filePath.Exists())
		return false;
	wxFile file(this->filePath.GetFullPath(), wxFile::OpenMode::read);
	if (!file.IsOpened())
		return false;
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

void SourceFileEditControl::ShowExecutionSuspendedAt(int lineNumber, int columnNumber)
{
	this->ClearExecutionMarker();
	this->suspensionMarkerHandler = this->MarkerAdd(lineNumber - 1, 1);
	this->ShowLineAndColumn(lineNumber, columnNumber);
}

void SourceFileEditControl::ShowLineAndColumn(int lineNumber, int columnNumber)
{
	this->GotoLine(lineNumber - 1);
	this->GotoPos(columnNumber - 1);
	int firstVisibleLine = this->GetFirstVisibleLine();
	int lastVisibleLine = firstVisibleLine + this->LinesOnScreen();
	if (lineNumber - 1 < firstVisibleLine || lineNumber - 1 > lastVisibleLine)
		this->ScrollToLine(lineNumber - 1);
}

void SourceFileEditControl::ClearExecutionMarker()
{
	if (this->suspensionMarkerHandler >= 0)
	{
		this->MarkerDeleteHandle(this->suspensionMarkerHandler);
		this->suspensionMarkerHandler = -1;
	}
}