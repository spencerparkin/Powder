#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class LiteralExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		LiteralExpressionHandler();
		virtual ~LiteralExpressionHandler();

		virtual void HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}