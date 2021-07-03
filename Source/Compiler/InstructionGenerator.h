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

		class SyntaxHandler
		{
		public:
			SyntaxHandler() {}
			virtual ~SyntaxHandler() {}

			virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) = 0;
		};

		void GenerateInstructionListRecursively(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode);

	private:

		HashMap<SyntaxHandler*> syntaxHandlerMap;
	};
}