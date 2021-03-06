#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ContainerFieldExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ContainerFieldExpressionHandler();
		virtual ~ContainerFieldExpressionHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}