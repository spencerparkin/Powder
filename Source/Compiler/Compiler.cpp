#include "Compiler.h"
#include "Tokenizer.h"
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

		//...

		LinkedList<Instruction*> instructionList;
		//...

		Assembler assembler;
		programBuffer = assembler.AssembleExecutable(instructionList, programBufferSize);
		
		return programBuffer;
	}
}