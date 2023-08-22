#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ReturnStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ReturnStatementHandler();
		virtual ~ReturnStatementHandler();

		virtual void HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}