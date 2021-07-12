#pragma once

#include <wx/textctrl.h>

// I'm hoping that program output as well as input can be managed by this control.
class TerminalControl : public wxTextCtrl
{
public:
	TerminalControl(wxWindow* parent);
	virtual ~TerminalControl();
};