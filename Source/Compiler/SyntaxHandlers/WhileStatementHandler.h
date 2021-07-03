#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class WhileStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		WhileStatementHandler();
		virtual ~WhileStatementHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}