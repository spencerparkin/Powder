#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ListPushPopExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ListPushPopExpressionHandler();
		virtual ~ListPushPopExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}