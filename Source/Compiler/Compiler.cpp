#include "Compiler.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "InstructionGenerator.h"
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
		Parser::SyntaxNode* rootSyntaxNode = parser.Parse(tokenList);

#if false
		if (rootSyntaxNode)
			rootSyntaxNode->Print(std::cout);
		else
			std::cout << "No syntax tree!" << std::endl;
#endif

		LinkedList<Instruction*> instructionList;
		InstructionGenerator instructionGenerator;
		instructionGenerator.GenerateInstructionList(instructionList, rootSyntaxNode);
		delete rootSyntaxNode;

		Assembler assembler;
		programBuffer = assembler.AssembleExecutable(instructionList, programBufferSize);
		return programBuffer;
	}
}