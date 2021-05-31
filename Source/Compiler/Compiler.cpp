#include "Compiler.h"
#include "Tokenizer.h"
#include "ProgramConstruct.h"
#include "Assembler.h"

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

		std::list<std::string> errorList;
		ProgramConstruct* programConstruct = new ProgramConstruct();
		LanguageConstruct::ParseResult parseResult = programConstruct->Parse(tokenList, errorList);
		if (parseResult != LanguageConstruct::ParseResult::SUCCESS)
		{
			//...
		}
		else
		{
			// TODO: Here we might do some sort of rule check on the resulting construct tree.
			//       Does it make sense?  For example, we can't have a function definition inside
			//       the conditional of an if-then-else statement.  That makes no sense.

			LinkedList<Instruction*> instructionList;
			programConstruct->GenerateInstructionSequence(instructionList);

			Assembler assembler;
			programBuffer = assembler.AssembleExecutable(instructionList, programBufferSize);
		}

		delete programConstruct;
		return programBuffer;
	}
}