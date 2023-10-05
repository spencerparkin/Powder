#include "SourceFileEditControl.h"
#include "SourceFileNotebookControl.h"
#include "EditorFrame.h"
#include "EditorApp.h"
#include "Lexer.h"
#include "StringTransformer.h"
#include <wx/file.h>

SourceFileEditControl::SourceFileEditControl(wxWindow* parent, const wxString& filePath) : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0L)
{
	this->filePath = filePath;
	this->modified = false;
	this->ready = false;
	this->suspensionMarkerHandler = -1;

	this->SetIndent(4);
	this->SetUseTabs(false);	// I actually prefer tabs, but the scintilla line/column calculations are easier if we're not using them.

	wxFont font;
	font.SetFaceName("Courier New");
	font.SetFamily(wxFONTFAMILY_MODERN);
	this->StyleSetFont(0, font);
	this->StyleSetFont(STYLE_COMMENT, font);
	this->StyleSetFont(STYLE_KEYWORD, font);
	this->StyleSetFont(STYLE_OPERATOR, font);
	this->StyleSetFont(STYLE_STRING, font);
	this->StyleSetFont(STYLE_NUMBER, font);

	this->SetMarginCount(2);
	this->SetMarginType(0, wxSTC_MARGIN_NUMBER);
	this->SetMarginWidth(0, 30);
	this->SetMarginSensitive(0, true);
	this->MarkerDefine(BREAKPOINT_MARKER, wxSTC_MARK_CIRCLE, *wxRED, *wxRED);
	this->MarkerDefine(EXECUTIONPOINT_MARKER, wxSTC_MARK_ARROW, *wxGREEN, *wxGREEN);
	this->SetLexer(wxSTC_LEX_CONTAINER);
	this->StyleSetForeground(STYLE_COMMENT, wxColour(0, 128, 0));
	this->StyleSetForeground(STYLE_KEYWORD, *wxBLUE);
	this->StyleSetForeground(STYLE_OPERATOR, *wxRED);
	this->StyleSetForeground(STYLE_STRING, wxColour(0, 128, 128));
	this->StyleSetForeground(STYLE_NUMBER, wxColour(128, 0, 128));

	this->Bind(wxEVT_STC_MODIFIED, &SourceFileEditControl::OnModified, this, wxID_ANY);
	this->Bind(wxEVT_STC_UPDATEUI, &SourceFileEditControl::OnUpdateUI, this, wxID_ANY);
	this->Bind(wxEVT_STC_MARGINCLICK, &SourceFileEditControl::OnMarginClicked, this, wxID_ANY);
	this->Bind(wxEVT_STC_STYLENEEDED, &SourceFileEditControl::OnStyleNeeded, this, wxID_ANY);
}

/*virtual*/ SourceFileEditControl::~SourceFileEditControl()
{
}

void SourceFileEditControl::OnStyleNeeded(wxStyledTextEvent& event)
{
	size_t startPos = this->GetEndStyled();
	size_t endPos = this->GetCurrentPos();

	// Always back-up to the start of the line.
	int line = this->LineFromPosition(startPos);
	startPos = this->FindColumn(line, 0);

	this->StyleRangeWithLexer(startPos, endPos);
}

void SourceFileEditControl::StyleAllWithLexer()
{
	int startPos = 0;
	int endPos = this->GetTextLength();
	this->StyleRangeWithLexer(startPos, endPos);
}

int SourceFileEditControl::TokenTypeToStyle(int tokenType)
{
	int style = 0;

	switch (tokenType)
	{
		case int(ParseParty::Lexer::Token::Type::IDENTIFIER_KEYWORD):
		{
			style = STYLE_KEYWORD;
			break;
		}
		case int(ParseParty::Lexer::Token::Type::COMMENT):
		{
			style = STYLE_COMMENT;
			break;
		}
		case int(ParseParty::Lexer::Token::Type::STRING_LITERAL):
		{
			style = STYLE_STRING;
			break;
		}
		case int(ParseParty::Lexer::Token::Type::OPERATOR):
		{
			style = STYLE_OPERATOR;
			break;
		}
		case int(ParseParty::Lexer::Token::Type::NUMBER_LITERAL_FLOAT):
		case int(ParseParty::Lexer::Token::Type::NUMBER_LITERAL_INT):
		{
			style = STYLE_NUMBER;
			break;
		}
	}

	return style;
}

void SourceFileEditControl::StyleRangeWithLexer(size_t startPos, size_t endPos)
{
	if (startPos >= endPos)
		return;

	wxString text = this->GetTextRange(startPos, endPos);

	ParseParty::Lexer::FileLocation initialFileLocation{ this->LineFromPosition(startPos) + 1, this->GetColumn(startPos) + 1 };

	std::vector<ParseParty::Lexer::Token*> tokenArray;
	std::string error;
	wxGetApp().lexer.tabSize = this->GetIndent();
	wxGetApp().lexer.Tokenize((const char*)text.c_str(), tokenArray, error, true, initialFileLocation);
	
	// Whether the tokenizer succeeded or failed, style what it was able to tokenize.
	for (ParseParty::Lexer::Token* token : tokenArray)
	{
		int style = this->TokenTypeToStyle(int(token->type));
		if (style != 0)
		{
			int stylePos = this->FindColumn(token->fileLocation.line - 1, token->fileLocation.column - 1);
			int styleLength = token->text->length();
			if (token->type == ParseParty::Lexer::Token::Type::STRING_LITERAL)
			{
				ParseParty::EspaceSequenceDecoder decoder;
				std::string stringText;
				decoder.Transform(*token->text, stringText);
				styleLength = stringText.length();
				styleLength += 2;	// Add 2 for the ommitted quotes.
			}

			this->StartStyling(stylePos);
			this->SetStyling(styleLength, style);
		}
	}

	for (ParseParty::Lexer::Token* token : tokenArray)
		delete token;
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

void SourceFileEditControl::OnUpdateUI(wxStyledTextEvent& event)
{
	int currentPos = this->GetCurrentPos();
	int lineNumber = 1 + this->LineFromPosition(currentPos);
	int columnNumber = this->GetColumn(currentPos);

	EditorFrame* frame = wxGetApp().GetFrame();
	if (frame)
	{
		wxStatusBar* statusBar = frame->GetStatusBar();
		if (statusBar)
			statusBar->SetStatusText(wxString::Format("Ln %d, Col %d", lineNumber, columnNumber));
	}
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

bool SourceFileEditControl::NewFile(void)
{
	if (this->filePath.Exists())
		return false;
	wxFile file(this->filePath.GetFullPath(), wxFile::OpenMode::write);
	if (!file.IsOpened())
		return false;
	wxString fileContents = "# " + this->filePath.GetName() + "." + this->filePath.GetExt() + "\n\n";
	file.Write(fileContents);
	file.Close();
	this->SetText(fileContents);
	this->StyleAllWithLexer();
	this->ready = true;
	this->modified = false;
	return true;
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
	file.Close();
	this->ConvertTabsToSpaces(fileContents);
	this->SetText(fileContents);
	this->StyleAllWithLexer();
	this->ready = true;
	return true;
}

bool SourceFileEditControl::SaveFile(void)
{
	wxFile file(this->filePath.GetFullPath(), wxFile::OpenMode::write);
	wxString fileContent = this->GetText();
	if (!file.Write(fileContent))
		return false;
	file.Close();
	this->modified = false;
	return true;
}

void SourceFileEditControl::ShowExecutionSuspendedAt(int lineNumber, int columnNumber)
{
	this->ClearExecutionMarker();
	this->suspensionMarkerHandler = this->MarkerAdd(lineNumber - 1, 1);
	this->EnsureLineAndColumnVisible(lineNumber, columnNumber);
}

void SourceFileEditControl::EnsureLineAndColumnVisible(int lineNumber, int columnNumber)
{
	int firstVisibleLine = this->GetFirstVisibleLine();
	int lastVisibleLine = firstVisibleLine + this->LinesOnScreen();
	if (lineNumber - 1 < firstVisibleLine || lineNumber - 1 > lastVisibleLine)
	{
		int position = this->PositionFromLine(lineNumber - 1) + columnNumber - 1;
		this->GotoPos(position);
	}
}

void SourceFileEditControl::ClearExecutionMarker()
{
	if (this->suspensionMarkerHandler >= 0)
	{
		this->MarkerDeleteHandle(this->suspensionMarkerHandler);
		this->suspensionMarkerHandler = -1;
	}
}

void SourceFileEditControl::ConvertTabsToSpaces(wxString& text)
{
	wxString convertedText;

	for (int i = 0; i < (signed)text.length(); i++)
	{
		char ch = text.GetChar(i);
		if (ch != '\t')
			convertedText += ch;
		else
		{
			for (int j = 0; j < (signed)this->GetIndent(); j++)
				convertedText += ' ';
		}
	}

	text = convertedText;
}