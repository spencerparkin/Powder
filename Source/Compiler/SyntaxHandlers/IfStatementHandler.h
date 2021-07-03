#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class IfStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		IfStatementHandler();
		virtual ~IfStatementHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}