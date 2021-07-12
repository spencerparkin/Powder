#include "TerminalControl.h"

TerminalControl::TerminalControl(wxWindow* parent) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT | wxTE_MULTILINE | wxHSCROLL)
{
}

/*virtual*/ TerminalControl::~TerminalControl()
{
}