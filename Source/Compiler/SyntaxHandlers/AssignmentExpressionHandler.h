#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class AssignmentExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		AssignmentExpressionHandler();
		virtual ~AssignmentExpressionHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;
	};
}