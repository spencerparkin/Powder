#include "Assembler.h"
#include "Instruction.h"
#include "Executable.h"
#include "JumpInstruction.h"
#include <iostream>

namespace Powder
{
	Assembler::Assembler()
	{
	}

	/*virtual*/ Assembler::~Assembler()
	{
	}

	bool Assembler::ResolveJumps(const LinkedList<Instruction*>& instructionList, Error& error)
	{
		// TODO: I should probably replace all uint64_t types with uint32_t, and int64_t with int32_t.
		std::vector<Instruction*> instructionArray;
		for (const LinkedList<Instruction*>::Node* node = instructionList.GetHead(); node; node = node->GetNext())
			instructionArray.push_back(node->value);
		for (int i = 0; i < (signed)instructionArray.size(); i++)
		{
			Instruction* instruction = instructionArray[i];
			const AssemblyData::Entry* jumpDeltaEntry = instruction->assemblyData->configMap.LookupPtr("jump-delta");
			if (jumpDeltaEntry)
			{
				int j = i + int(jumpDeltaEntry->jumpDelta);
				AssemblyData::Entry entry;
				if (j < 0 || j >(signed)instructionArray.size())
				{
					error.Add(std::format("Instruction at {} wants to jump to the instruction at {}, but there are only {} instructions.", i, j, instructionArray.size()));
					return false;
				}
				entry.instruction = instructionArray[j];
				instruction->assemblyData->configMap.Insert(jumpDeltaEntry->string.c_str(), entry);
			}
		}

		return true;
	}

	Executable* Assembler::AssembleExecutable(const LinkedList<Instruction*>& instructionList, bool generateDebugInfo, Error& error)
	{
		if (!this->ResolveJumps(instructionList, error))
			return nullptr;

		uint64_t programBufferLocation = 0L;
		for(const LinkedList<Instruction*>::Node* node = instructionList.GetHead(); node; node = node->GetNext())
		{
			const Instruction* instruction = node->value;
			instruction->assemblyData->programBufferLocation = programBufferLocation;
			if (!instruction->Assemble(nullptr, programBufferLocation, Instruction::AssemblyPass::CALC_EXTENT, error))
				return nullptr;
		}

		if (programBufferLocation == 0L)
			return nullptr;

		Executable* executable = new Executable();
		executable->byteCodeBufferSize = programBufferLocation;
		executable->byteCodeBuffer = new uint8_t[(uint32_t)executable->byteCodeBufferSize];

		ParseParty::JsonObject* instructionMapValue = nullptr;

		if (generateDebugInfo)
		{
			executable->debugInfoDoc = new ParseParty::JsonObject();
			instructionMapValue = new ParseParty::JsonObject();
			executable->debugInfoDoc->SetValue("instruction_map", instructionMapValue);
		}

		programBufferLocation = 0L;
		for(const LinkedList<Instruction*>::Node* node = instructionList.GetHead(); node; node = node->GetNext())
		{
			const Instruction* instruction = node->value;
			executable->byteCodeBuffer[programBufferLocation] = instruction->OpCode();

			if (generateDebugInfo)
			{
				ParseParty::JsonObject* instructionMapEntryValue = new ParseParty::JsonObject();
				instructionMapEntryValue->SetValue("instruction", new ParseParty::JsonString(instruction->Print()));
				instructionMapEntryValue->SetValue("line", new ParseParty::JsonInt(instruction->assemblyData->fileLocation.line));
				instructionMapEntryValue->SetValue("col", new ParseParty::JsonInt(instruction->assemblyData->fileLocation.column));
				if (instruction->assemblyData->debuggerHelp->length() > 0)
					instructionMapEntryValue->SetValue("debugger_help", new ParseParty::JsonString(instruction->assemblyData->debuggerHelp->c_str()));
				std::string addressStr = std::format("{}", programBufferLocation);
				instructionMapValue->SetValue(addressStr, instructionMapEntryValue);
			}

			if (!instruction->Assemble(executable, programBufferLocation, Instruction::AssemblyPass::RENDER, error))
			{
				delete executable;
				return nullptr;
			}
		}

		return executable;
	}
}