#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ComprehensionExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ComprehensionExpressionHandler();
		virtual ~ComprehensionExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}