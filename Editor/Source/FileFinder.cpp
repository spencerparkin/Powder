#include "FileFinder.h"
#include <wx/filename.h>

FileFinder::FileFinder()
{
}

/*virtual*/ FileFinder::~FileFinder()
{
}

/*virtual*/ wxDirTraverseResult FileFinder::OnFile(const wxString& fileName)
{
	wxString fileExt = wxFileName(fileName).GetExt();

	for (int i = 0; i < (signed)this->extensionArray.size(); i++)
	{
		const wxString& matchExt = this->extensionArray[i];
		if (fileExt == matchExt)
		{
			this->foundFileArray.Add(fileName);
			break;
		}
	}

	return wxDIR_CONTINUE;
}

/*virtual*/ wxDirTraverseResult FileFinder::OnDir(const wxString& dirName)
{
	return wxDIR_CONTINUE;
}