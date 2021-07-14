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
	wxString GetFileName();

	enum
	{
		BREAKPOINT_MARKER,
		EXECUTIONPOINT_MARKER
	};

	void OnModified(wxStyledTextEvent& event);
	void OnKeyPressed(wxStyledTextEvent& event);
	void OnMarginClicked(wxStyledTextEvent& event);

	void ShowExecutionSuspendedAt(int lineNumber, int columnNumber);
	void ShowLineAndColumn(int lineNumber, int columnNumber);
	void ClearExecutionMarker();
	void UpdateBreakpointMarkers();

	wxFileName filePath;
	bool modified;
	bool ready;
	int suspensionMarkerHandler;
};