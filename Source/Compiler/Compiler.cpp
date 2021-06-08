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
		
#if defined POWDER_DEBUG
		if (rootSyntaxNode)
			rootSyntaxNode->Print(std::cout);
		else
			std::cout << "No syntax tree!" << std::endl;
#endif

		if (!rootSyntaxNode)
			throw new CompileTimeException("Failed to generate AST from tokenization of the source code.");

		LinkedList<Instruction*> instructionList;
		HashMap<Instruction*> functionMap;
		InstructionGenerator instructionGenerator;
		instructionGenerator.GenerateInstructionList(instructionList, functionMap, rootSyntaxNode);
		delete rootSyntaxNode;

		Assembler assembler;
		programBuffer = assembler.AssembleExecutable(instructionList, functionMap, programBufferSize);
		return programBuffer;
	}
}