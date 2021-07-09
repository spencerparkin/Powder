#pragma once

#include <wx/stc/stc.h>
#include <wx/filename.h>

class SourceFileEditControl : public wxStyledTextCtrl
{
public:
	SourceFileEditControl(wxWindow* parent, const wxString& filePath);
	virtual ~SourceFileEditControl();

	bool LoadFile(void);
	bool SaveFile(void);

	wxString GetTabLabel();
	wxString GetFilename();

	void OnTextChanged(wxStyledTextEvent& event);

	wxFileName filePath;
	bool modified;
	bool ready;
};