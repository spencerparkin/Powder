#include "ArtProvider.h"

ArtProvider::ArtProvider(const wxString& baseFolder)
{
	this->baseFolder = baseFolder;
}

/*virtual*/ ArtProvider::~ArtProvider()
{
}

/*virtual*/ wxBitmap ArtProvider::CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size)
{
	wxString bitmapFile;

	if (id == ART_EDITOR_APP_ICON)
		bitmapFile = "AppIcon.png";
	else if (id == ART_EDITOR_RUN_WITH_DEBUGGER)
		bitmapFile = "RunWithDebuggerIcon.png";
	else if (id == ART_EDITOR_RUN_WITHOUT_DEBUGGER)
		bitmapFile = "RunIcon.png";
	else if (id == ART_EDITOR_KILL_SCRIPT)
		bitmapFile = "KillIcon.png";
	else if (id == ART_EDITOR_PAUSE_SCRIPT)
		bitmapFile = "BreakIcon.png";
	else if (id == ART_EDITOR_RESUME_SCRIPT)
		bitmapFile = "RunIcon.png";
	else if (id == ART_EDITOR_STEP_INTO)
		bitmapFile = "StepIntoIcon.png";
	else if (id == ART_EDITOR_STEP_OUT)
		bitmapFile = "StepOutIcon.png";
	else if (id == ART_EDITOR_STEP_OVER)
		bitmapFile = "StepOverIcon.png";
	else if (id == ART_EDITOR_DELETE_TARGETS)
		bitmapFile = "BombIcon.png";

	if (bitmapFile.Length() > 0)
	{
		wxString bitmapFullPath = this->baseFolder + "/Icons/" + bitmapFile;
		if (wxFileExists(bitmapFullPath))
		{
			wxBitmap bitmap;
			if (bitmap.LoadFile(bitmapFullPath, wxBITMAP_TYPE_PNG))
				return bitmap;
		}
	}

	return wxArtProvider::CreateBitmap(id, client, size);
}