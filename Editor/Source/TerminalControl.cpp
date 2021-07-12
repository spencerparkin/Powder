#include "TerminalControl.h"
#include "EditorApp.h"
#include "EditorFrame.h"

wxDEFINE_EVENT(EVT_TERMINAL_INPUT_READY, wxCommandEvent);

TerminalControl::TerminalControl(wxWindow* parent) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT | wxTE_MULTILINE | wxHSCROLL)
{
	this->inputText = nullptr;
	this->readOnly = true;
	this->Bind(wxEVT_CHAR, &TerminalControl::OnCharPressed, this);
}

/*virtual*/ TerminalControl::~TerminalControl()
{
}

void TerminalControl::EditString(wxString* inputText)
{
	this->readOnly = false;
	this->inputText = inputText;
}

void TerminalControl::OnCharPressed(wxKeyEvent& event)
{
	if (this->readOnly)
		return;

	if (this->inputText)
	{
		if (event.GetKeyCode() == '\r')
		{
			this->inputText = nullptr;
			::wxQueueEvent(wxGetApp().GetFrame(), new wxCommandEvent(EVT_TERMINAL_INPUT_READY));
		}
		else if (event.GetKeyCode() == wxKeyCode::WXK_BACK)
		{
			if (this->inputText->length() > 0)
				this->inputText->Truncate(this->inputText->length() - 1);
			else
				return;
		}
		else
		{
			char ch = (char)event.GetKeyCode();
			if (::isalnum(ch))
				*this->inputText += ch;
		}
	}

	event.Skip();
}