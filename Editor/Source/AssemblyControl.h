#pragma once

#include <wx/listctrl.h>
#include "JsonValue.h"

class AssemblyControl : public wxListCtrl
{
public:
	AssemblyControl(wxWindow* parent);
	virtual ~AssemblyControl();

	virtual wxString OnGetItemText(long item, long column) const override;

	void Clear();
	void OnExecutionSuspended(uint64_t programBufferLocation, const ParseParty::JsonObject* instructionMap);

	struct InstructionEntry
	{
		uint64_t address;
		wxString instruction;
	};

	std::vector<InstructionEntry> instructionList;
	std::map<uint64_t, int> addressMap;
};