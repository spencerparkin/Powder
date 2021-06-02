#include "Compiler.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "Assembler.h"
#include "Exceptions.hpp"
#include <iostream>

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

		Parser parser;
		Parser::SyntaxNode* syntaxNode = parser.Parse(tokenList);

		if (syntaxNode)
			syntaxNode->Print(std::cout);
		else
			std::cout << "No syntax tree!" << std::endl;

		LinkedList<Instruction*> instructionList;
		
		// TODO: Generate instruction list as a function of the syntax tree.

		delete syntaxNode;

		Assembler assembler;
		programBuffer = assembler.AssembleExecutable(instructionList, programBufferSize);
		
		return programBuffer;
	}
}