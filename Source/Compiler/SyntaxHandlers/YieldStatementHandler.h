#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class YieldStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		YieldStatementHandler();
		virtual ~YieldStatementHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}