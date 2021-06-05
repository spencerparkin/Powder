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

		void GenerateInstructionList(LinkedList<Instruction*>& instructionList, HashMap<Instruction*>& functionMap, const Parser::SyntaxNode* rootSyntaxNode);

	private:

		void GenerateInstructionListRecursively(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode);
		void GatherAllFunctionSignatures(const Parser::SyntaxNode* syntaxNode);
		void GenerateFunctionReturnInstructions(LinkedList<Instruction*>& instructionList);

		LinkedList<const Parser::SyntaxNode*> functionDefinitionList;

		struct FunctionSignature
		{
			std::vector<std::string> namedArgsArray;
		};

		HashMap<FunctionSignature*> functionSignatureMap;
	};
}