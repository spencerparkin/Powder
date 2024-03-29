#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class MapExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		MapExpressionHandler();
		virtual ~MapExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}