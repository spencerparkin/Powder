#pragma once

#include "Defines.h"
#include "LinkedList.hpp"
#include "Parser.h"
#include "HashMap.hpp"
#include <vector>

namespace Powder
{
	class Instruction;
	class Error;

	class POWDER_API InstructionGenerator
	{
	public:
		InstructionGenerator();
		virtual ~InstructionGenerator();

		bool GenerateInstructionList(LinkedList<Instruction*>& instructionList, const ParseParty::Parser::SyntaxNode* rootSyntaxNode, Error& error);

		class SyntaxHandler
		{
		public:
			SyntaxHandler() {}
			virtual ~SyntaxHandler() {}

			virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) = 0;

			bool PopNeededForExpression(const ParseParty::Parser::SyntaxNode* syntaxNode);
			void FindBreakAndContinueJumps(const ParseParty::Parser::SyntaxNode* syntaxNode, const LinkedList<Instruction*>& loopBodyInstructions, LinkedList<Instruction*>& breakInstructionList, LinkedList<Instruction*>& continueInstructionList);
		};

		bool GenerateInstructionListRecursively(LinkedList<Instruction*>& instructionList, const ParseParty::Parser::SyntaxNode* syntaxNode, Error& error);

	private:

		HashMap<SyntaxHandler*> syntaxHandlerMap;
	};
}