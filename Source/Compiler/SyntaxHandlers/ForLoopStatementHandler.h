#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ForLoopStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ForLoopStatementHandler();
		virtual ~ForLoopStatementHandler();

		virtual void HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;

		void GenerateInstructionForIteratorCallSetup(LinkedList<Instruction*>& instructionList, const char* action, const ParseParty::Lexer::FileLocation& fileLocation);
	};
}