#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ContainerSizeExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ContainerSizeExpressionHandler();
		virtual ~ContainerSizeExpressionHandler();

		virtual void HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}