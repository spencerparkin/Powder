#include "Assembler.h"
#include "Instruction.h"

namespace Powder
{
	Assembler::Assembler()
	{
	}

	/*virtual*/ Assembler::~Assembler()
	{
	}

	uint8_t* Assembler::AssembleExecutable(const LinkedList<Instruction*>& instructionList, uint64_t& programBufferSize)
	{
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