#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class IdentifierExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		IdentifierExpressionHandler();
		virtual ~IdentifierExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}