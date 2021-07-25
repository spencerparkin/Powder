#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ForLoopStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ForLoopStatementHandler();
		virtual ~ForLoopStatementHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}