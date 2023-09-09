#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class StatementListHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		StatementListHandler();
		virtual ~StatementListHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}