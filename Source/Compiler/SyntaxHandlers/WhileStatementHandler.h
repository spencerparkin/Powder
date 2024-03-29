#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class WhileStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		WhileStatementHandler();
		virtual ~WhileStatementHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}