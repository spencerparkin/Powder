#pragma once

#include <wx/aui/auibook.h>
#include <wx/filename.h>

class SourceFileEditControl;

class SourceFileNotebookControl : public wxAuiNotebook
{
public:
	SourceFileNotebookControl(wxWindow* parent);
	virtual ~SourceFileNotebookControl();

	void OnPageCloseButtonPushed(wxAuiNotebookEvent& event);

	bool OpenSourceFile(const wxString& filePath);
	bool CloseSourceFile(int pageNumber);
	void SaveSourceFile(int pageNumber);
	bool CloseAllFiles(void);
	void SaveAllFiles(void);
	void UpdateTabLabelFor(SourceFileEditControl* editControl);
	bool AnyFilesModified();
	int OpenFileCount();
	bool CanClosePage(int pageNumber);
	SourceFileEditControl* FindEditControl(const wxFileName& filePath, int* pageNumber = nullptr);
	SourceFileEditControl* GetSelectedEditControl();
	void RememberCurrentlyOpenFiles();
	void RestoreOpenFiles();
	void ClearExecutionMarkers();
};