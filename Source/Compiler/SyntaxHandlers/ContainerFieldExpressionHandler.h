#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ContainerFieldExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ContainerFieldExpressionHandler();
		virtual ~ContainerFieldExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}