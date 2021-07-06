#include "Assembler.h"
#include "Instruction.h"
#include "StringFormat.h"
#include "Executable.h"
#include "JumpInstruction.h"
#include "Exceptions.hpp"
#include <iostream>

namespace Powder
{
	Assembler::Assembler()
	{
	}

	/*virtual*/ Assembler::~Assembler()
	{
	}

	void Assembler::ResolveJumps(const LinkedList<Instruction*>& instructionList)
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
					throw new CompileTimeException(FormatString("Instruction at %d wants to jump to the instruction at %d, but there are only %d instructions.", i, j, instructionArray.size()));
				entry.instruction = instructionArray[j];
				instruction->assemblyData->configMap.Insert(jumpDeltaEntry->string.c_str(), entry);
			}
		}
	}

	Executable* Assembler::AssembleExecutable(const LinkedList<Instruction*>& instructionList)
	{
		this->ResolveJumps(instructionList);

		uint64_t programBufferLocation = 0L;
		for(const LinkedList<Instruction*>::Node* node = instructionList.GetHead(); node; node = node->GetNext())
		{
			const Instruction* instruction = node->value;
			instruction->assemblyData->programBufferLocation = programBufferLocation;
			instruction->Assemble(nullptr, programBufferLocation, Instruction::AssemblyPass::CALC_EXTENT);
		}

		if (programBufferLocation == 0L)
			return nullptr;

		Executable* executable = new Executable();
		executable->byteCodeBufferSize = programBufferLocation;
		executable->byteCodeBuffer = new uint8_t[(uint32_t)executable->byteCodeBufferSize];

		programBufferLocation = 0L;
		for(const LinkedList<Instruction*>::Node* node = instructionList.GetHead(); node; node = node->GetNext())
		{
			const Instruction* instruction = node->value;
			executable->byteCodeBuffer[programBufferLocation] = instruction->OpCode();
#if defined POWDER_DEBUG
			std::cout << FormatString("%04d: ", programBufferLocation) << instruction->Print() << std::endl;
#endif
			instruction->Assemble(executable, programBufferLocation, Instruction::AssemblyPass::RENDER);
		}

		return executable;
	}
}