#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class BinaryExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		BinaryExpressionHandler();
		virtual ~BinaryExpressionHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}