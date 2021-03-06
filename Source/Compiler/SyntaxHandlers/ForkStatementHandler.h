#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ForkStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ForkStatementHandler();
		virtual ~ForkStatementHandler();

		virtual void HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;
	};
}
