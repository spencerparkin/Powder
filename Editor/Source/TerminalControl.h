#pragma once

#include <wx/textctrl.h>

wxDECLARE_EVENT(EVT_TERMINAL_INPUT_READY, wxCommandEvent);

class TerminalControl : public wxTextCtrl
{
public:
	TerminalControl(wxWindow* parent);
	virtual ~TerminalControl();

	void OnCharPressed(wxKeyEvent& event);
	void EditString(wxString* inputText);

private:

	wxString* inputText;
	bool readOnly;
};