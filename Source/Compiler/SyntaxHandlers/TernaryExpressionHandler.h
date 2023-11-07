#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class TernaryExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		TernaryExpressionHandler();
		virtual ~TernaryExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}
