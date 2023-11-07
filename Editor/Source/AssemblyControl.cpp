#include "AssemblyControl.h"

AssemblyControl::AssemblyControl(wxWindow* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_VIRTUAL | wxLC_REPORT | wxLC_SINGLE_SEL)
{
	this->AppendColumn("Address");
	this->AppendColumn("Instruction");
}

/*virtual*/ AssemblyControl::~AssemblyControl()
{
}

/*virtual*/ wxString AssemblyControl::OnGetItemText(long item, long column) const
{
	if (item < 0 || item >= (signed)this->instructionList.size())
		return "";

	const InstructionEntry& entry = this->instructionList[item];
	switch (column)
	{
		case 0: return wxString::Format("0x%p", (void*)entry.address);
		case 1: return entry.instruction;
	}

	return "?";
}

void AssemblyControl::Clear()
{
	this->instructionList.clear();
	this->SetItemCount(0);
	this->addressMap.clear();
	this->Refresh();
}

void AssemblyControl::OnExecutionSuspended(uint64_t programBufferLocation, const ParseParty::JsonObject* instructionMap)
{
	if (this->instructionList.size() == 0)
	{
		for (std::pair<std::string, ParseParty::JsonValue*> pair : *instructionMap)
		{
			const ParseParty::JsonObject* jsonEntry = dynamic_cast<const ParseParty::JsonObject*>(pair.second);
			if (!jsonEntry)
				continue;
			
			const ParseParty::JsonString* jsonInstruction = dynamic_cast<const ParseParty::JsonString*>(jsonEntry->GetValue("instruction"));
			if (!jsonInstruction)
				continue;

			InstructionEntry entry;
			entry.address = ::atol(pair.first.c_str());
			entry.instruction = jsonInstruction->GetValue();
			this->instructionList.push_back(entry);
		}

		std::sort(this->instructionList.begin(), this->instructionList.end(), [](const InstructionEntry& entryA, const InstructionEntry& entryB) -> bool
			{
				return entryA.address < entryB.address;
			});

		this->SetItemCount(this->instructionList.size());

		addressMap.clear();
		for (int i = 0; i < (signed)this->instructionList.size(); i++)
		{
			const InstructionEntry& entry = this->instructionList[i];
			addressMap.insert(std::pair<uint64_t, int>(entry.address, i));
		}

		this->SetColumnWidth(0, wxLIST_AUTOSIZE);
		this->SetColumnWidth(1, wxLIST_AUTOSIZE);
	}

	std::map<uint64_t, int>::iterator iter = addressMap.find(programBufferLocation);
	if (iter != addressMap.end())
	{
		bool stateChanged = this->SetItemState(iter->second, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		wxASSERT(stateChanged);

		this->EnsureVisible(iter->second);
	}

	this->Refresh();
}