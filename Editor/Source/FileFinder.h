#pragma once

#include <wx/dir.h>

class FileFinder : public wxDirTraverser
{
public:
	FileFinder();
	virtual ~FileFinder();

	virtual wxDirTraverseResult OnFile(const wxString& fileName) override;
	virtual wxDirTraverseResult OnDir(const wxString& dirName) override;

	wxArrayString extensionArray;
	wxArrayString foundFileArray;
};