#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ListExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ListExpressionHandler();
		virtual ~ListExpressionHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}