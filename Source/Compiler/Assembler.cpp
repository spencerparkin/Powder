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

	uint8_t* Assembler::AssembleExecutable(const std::list<Instruction*>& instructionList, uint64_t& programBufferSize)
	{
		uint64_t programBufferLocation = 0L;
		for (std::list<Instruction*>::const_iterator iter = instructionList.cbegin(); iter != instructionList.cend(); iter++)
		{
			const Instruction* instruction = *iter;
			instruction->assemblyData->programBufferLocation = programBufferLocation;
			instruction->Assemble(nullptr, 0, programBufferLocation, Instruction::AssemblyPass::CALC_EXTENT);
		}

		programBufferSize = programBufferLocation;
		if (programBufferSize == 0L)
			return nullptr;

		uint8_t* programBuffer = new uint8_t[(unsigned int)programBufferSize];
		programBufferLocation = 0L;
		for (std::list<Instruction*>::const_iterator iter = instructionList.cbegin(); iter != instructionList.cend(); iter++)
		{
			const Instruction* instruction = *iter;
			programBuffer[programBufferLocation] = instruction->OpCode();
			instruction->Assemble(programBuffer, programBufferSize, programBufferLocation, Instruction::AssemblyPass::RENDER);
		}

		return programBuffer;
	}
}