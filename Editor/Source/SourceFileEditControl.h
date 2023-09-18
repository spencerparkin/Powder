#pragma once

#include <wx/stc/stc.h>
#include <wx/filename.h>

class SourceFileEditControl : public wxStyledTextCtrl
{
public:
	SourceFileEditControl(wxWindow* parent, const wxString& filePath);
	virtual ~SourceFileEditControl();

	bool NewFile(void);
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
	void OnUpdateUI(wxStyledTextEvent& event);
	void OnMarginClicked(wxStyledTextEvent& event);
	void OnStyleNeeded(wxStyledTextEvent& event);

	void ShowExecutionSuspendedAt(int lineNumber, int columnNumber);
	void ShowLineAndColumn(int lineNumber, int columnNumber);
	void ClearExecutionMarker();
	void UpdateBreakpointMarkers();
	void StyleRangeWithLexer(size_t startPos, size_t endPos);
	void StyleAllWithLexer();
	void ConvertTabsToSpaces(wxString& text);

	enum Style
	{
		STYLE_COMMENT = 10,
		STYLE_KEYWORD,
		STYLE_OPERATOR,
		STYLE_STRING,
		STYLE_NUMBER
	};

	int TokenTypeToStyle(int tokenType);

	wxFileName filePath;
	bool modified;
	bool ready;
	int suspensionMarkerHandler;
};