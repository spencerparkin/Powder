#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class MembershipExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		MembershipExpressionHandler();
		virtual ~MembershipExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}