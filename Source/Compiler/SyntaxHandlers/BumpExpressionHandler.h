#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class BumpExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		BumpExpressionHandler();
		virtual ~BumpExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}