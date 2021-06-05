#include "Assembler.h"
#include "Instruction.h"
#include "StringFormat.h"
#include "JumpInstruction.h"
#include "Exceptions.hpp"

namespace Powder
{
	Assembler::Assembler()
	{
	}

	/*virtual*/ Assembler::~Assembler()
	{
	}

	void Assembler::ResolveJumps(const LinkedList<Instruction*>& instructionList, const HashMap<Instruction*>& functionMap)
	{
		// Resolve all function call jumps.
		for (const LinkedList<Instruction*>::Node* node = instructionList.GetHead(); node; node = node->GetNext())
		{
			Instruction* instruction = node->value;
			const AssemblyData::Entry* jumpFuncEntry = instruction->assemblyData->configMap.LookupPtr("jump-func");
			if (jumpFuncEntry)
			{
				JumpInstruction* jumpInstruction = dynamic_cast<JumpInstruction*>(instruction);
				if (jumpInstruction)
				{
					std::string funcName = jumpFuncEntry->string;
					Instruction* functionFirstInstruction = const_cast<HashMap<Instruction*>*>(&functionMap)->Lookup(funcName.c_str());
					if (!functionFirstInstruction)
						throw new CompileTimeException(FormatString("Tried to call undefined function \"%s\".", funcName.c_str()), &jumpInstruction->assemblyData->fileLocation);

					AssemblyData::Entry entry;
					entry.instruction = functionFirstInstruction;
					jumpInstruction->assemblyData->configMap.Insert("jump", entry);
				}
			}
		}

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
					throw new CompileTimeException(FormatString("Instruction at %d wants to jump to the instruction at %d, but there are only %d instructions.", i, j, instructionArray.size()));
				entry.instruction = instructionArray[j];
				instruction->assemblyData->configMap.Insert(jumpDeltaEntry->string.c_str(), entry);
			}
		}
	}

	uint8_t* Assembler::AssembleExecutable(const LinkedList<Instruction*>& instructionList, const HashMap<Instruction*>& functionMap, uint64_t& programBufferSize)
	{
		this->ResolveJumps(instructionList, functionMap);

		uint64_t programBufferLocation = 0L;
		for(const LinkedList<Instruction*>::Node* node = instructionList.GetHead(); node; node = node->GetNext())
		{
			const Instruction* instruction = node->value;
			instruction->assemblyData->programBufferLocation = programBufferLocation;
			instruction->Assemble(nullptr, 0, programBufferLocation, Instruction::AssemblyPass::CALC_EXTENT);
		}

		programBufferSize = programBufferLocation;
		if (programBufferSize == 0L)
			return nullptr;

		uint8_t* programBuffer = new uint8_t[(unsigned int)programBufferSize];
		programBufferLocation = 0L;
		for(const LinkedList<Instruction*>::Node* node = instructionList.GetHead(); node; node = node->GetNext())
		{
			const Instruction* instruction = node->value;
			programBuffer[programBufferLocation] = instruction->OpCode();
			instruction->Assemble(programBuffer, programBufferSize, programBufferLocation, Instruction::AssemblyPass::RENDER);
		}

		return programBuffer;
	}
}