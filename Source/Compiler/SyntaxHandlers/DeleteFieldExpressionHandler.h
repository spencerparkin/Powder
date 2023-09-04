#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class DeleteFieldExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		DeleteFieldExpressionHandler();
		virtual ~DeleteFieldExpressionHandler();

		virtual void HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}