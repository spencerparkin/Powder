#pragma once

#include "Defines.h"
#include "LinkedList.hpp"
#include "Parser.h"
#include "HashMap.hpp"
#include <vector>

namespace Powder
{
	class Instruction;

	class POWDER_API InstructionGenerator
	{
	public:
		InstructionGenerator();
		virtual ~InstructionGenerator();

		void GenerateInstructionList(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* rootSyntaxNode);

	private:

		void GenerateInstructionListRecursively(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode);
		void GenerateFunctionReturnInstructions(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode);
		void GenerateFunctionArgumentInstructions(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode, const Parser::SyntaxNode* argListNode);

		struct FunctionDef
		{
			const Parser::SyntaxNode* syntaxNode;
			Instruction* declareInstruction;
		};

		LinkedList<FunctionDef> functionDefinitionList;
	};
}