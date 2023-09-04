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

		void GenerateInstructionList(LinkedList<Instruction*>& instructionList, const ParseParty::Parser::SyntaxNode* rootSyntaxNode);

		class SyntaxHandler
		{
		public:
			SyntaxHandler() {}
			virtual ~SyntaxHandler() {}

			virtual void HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) = 0;

			bool PopNeededForExpression(const ParseParty::Parser::SyntaxNode* syntaxNode);
		};

		void GenerateInstructionListRecursively(LinkedList<Instruction*>& instructionList, const ParseParty::Parser::SyntaxNode* syntaxNode);

	private:

		HashMap<SyntaxHandler*> syntaxHandlerMap;
	};
}