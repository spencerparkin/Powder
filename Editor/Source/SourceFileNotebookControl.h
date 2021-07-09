#pragma once

#include <wx/notebook.h>
#include <wx/filename.h>

class SourceFileEditControl;

class SourceFileNotebookControl : public wxNotebook
{
public:
	SourceFileNotebookControl(wxWindow* parent);
	virtual ~SourceFileNotebookControl();

	void ToggleSourceFile(const wxString& filePath);
	bool OpenSourceFile(const wxString& filePath);
	bool CloseSourceFile(int pageNumber);
	void SaveSourceFile(int pageNumber);
	bool CloseAllFiles(void);
	void SaveAllFiles(void);
	void UpdateTabLabelFor(SourceFileEditControl* editControl);
	bool AnyFilesModified();
	int OpenFileCount();
	SourceFileEditControl* FindEditControl(const wxFileName& filePath, int* pageNumber = nullptr);
};