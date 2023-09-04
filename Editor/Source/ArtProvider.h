#pragma once

#include <wx/artprov.h>

#define ART_EDITOR_RUN_WITH_DEBUGGER		wxART_MAKE_ART_ID(ART_EDITOR_RUN_WITH_DEBUGGER)
#define ART_EDITOR_RUN_WITHOUT_DEBUGGER		wxART_MAKE_ART_ID(ART_EDITOR_RUN_WITHOUT_DEBUGGER)
#define ART_EDITOR_STEP_OVER				wxART_MAKE_ART_ID(ART_EDITOR_STEP_OVER)
#define ART_EDITOR_STEP_INTO				wxART_MAKE_ART_ID(ART_EDITOR_STEP_INTO)
#define ART_EDITOR_STEP_OUT					wxART_MAKE_ART_ID(ART_EDITOR_STEP_OUT)
#define ART_EDITOR_PAUSE_SCRIPT				wxART_MAKE_ART_ID(ART_EDITOR_PAUSE_SCRIPT)
#define ART_EDITOR_RESUME_SCRIPT			wxART_MAKE_ART_ID(ART_EDITOR_RESUME_SCRIPT)
#define ART_EDITOR_KILL_SCRIPT				wxART_MAKE_ART_ID(ART_EDITOR_KILL_SCRIPT)
#define ART_EDITOR_DELETE_TARGETS			wxART_MAKE_ART_ID(ART_EDITOR_DELETE_TARGETS)

class ArtProvider : public wxArtProvider
{
public:
	ArtProvider(const wxString& baseFolder);
	virtual ~ArtProvider();

	virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size) override;

	wxString baseFolder;
};