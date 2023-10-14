#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class SetExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		SetExpressionHandler();
		virtual ~SetExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}