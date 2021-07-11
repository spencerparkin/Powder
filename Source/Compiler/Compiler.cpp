#include "Compiler.h"
#include "Value.h"
#include "Scope.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "InstructionGenerator.h"
#include "Assembler.h"
#include "Exceptions.hpp"
#include <iostream>

namespace Powder
{
	Compiler theDefaultCompiler;

	Compiler::Compiler()
	{
		this->generateDebugInfo = true;
	}

	/*virtual*/ Compiler::~Compiler()
	{
	}

	/*virtual*/ Executable* Compiler::CompileCode(const char* programSourceCode)
	{
		TokenList tokenList;
		Tokenizer tokenizer;
		tokenizer.Tokenize(programSourceCode, tokenList);

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
		InstructionGenerator instructionGenerator;
		instructionGenerator.GenerateInstructionList(instructionList, rootSyntaxNode);
		delete rootSyntaxNode;

		Assembler assembler;
		return assembler.AssembleExecutable(instructionList, this->generateDebugInfo);
	}
}