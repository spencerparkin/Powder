#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class BreakOrContinueStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		BreakOrContinueStatementHandler();
		virtual ~BreakOrContinueStatementHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}