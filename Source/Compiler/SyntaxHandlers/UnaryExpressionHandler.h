#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class UnaryExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		UnaryExpressionHandler();
		virtual ~UnaryExpressionHandler();

		virtual void HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}