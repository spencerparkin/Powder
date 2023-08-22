#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class MapKeysExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		MapKeysExpressionHandler();
		virtual ~MapKeysExpressionHandler();

		virtual void HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}