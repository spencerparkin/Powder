#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ReturnStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ReturnStatementHandler();
		virtual ~ReturnStatementHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}