#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class FunctionCallExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		FunctionCallExpressionHandler();
		virtual ~FunctionCallExpressionHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}