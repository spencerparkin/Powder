#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class MapExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		MapExpressionHandler();
		virtual ~MapExpressionHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}