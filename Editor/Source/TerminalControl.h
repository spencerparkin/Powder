#pragma once

#include <wx/textctrl.h>

class TerminalControl : public wxTextCtrl
{
public:
	TerminalControl(wxWindow* parent);
	virtual ~TerminalControl();
};