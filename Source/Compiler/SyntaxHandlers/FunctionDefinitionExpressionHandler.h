#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class FunctionDefinitionExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		FunctionDefinitionExpressionHandler();
		virtual ~FunctionDefinitionExpressionHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}