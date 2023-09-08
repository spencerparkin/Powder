#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class DoWhileStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		DoWhileStatementHandler();
		virtual ~DoWhileStatementHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}