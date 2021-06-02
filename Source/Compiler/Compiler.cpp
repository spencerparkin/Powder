#include "Compiler.h"
#include "Tokenizer.h"
#include "ProgramConstruct.h"
#include "Assembler.h"
#include "Exceptions.hpp"

namespace Powder
{
	Compiler::Compiler()
	{
	}

	/*virtual*/ Compiler::~Compiler()
	{
	}

	uint8_t* Compiler::CompileCode(const char* programCode, uint64_t& programBufferSize)
	{
		programBufferSize = 0L;
		uint8_t* programBuffer = nullptr;

		TokenList tokenList;
		Tokenizer tokenizer;
		tokenizer.Tokenize(programCode, tokenList);

		ProgramConstruct* programConstruct = new ProgramConstruct();
		if (!programConstruct->Parse(tokenList))
			throw new CompileTimeException("Failed to parse program at root level as program construct.", 0);

		LinkedList<Instruction*> instructionList;
		programConstruct->GenerateInstructionSequence(instructionList);

		delete programConstruct;

		Assembler assembler;
		programBuffer = assembler.AssembleExecutable(instructionList, programBufferSize);
		
		return programBuffer;
	}
}