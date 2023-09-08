#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class BinaryExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		BinaryExpressionHandler();
		virtual ~BinaryExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}