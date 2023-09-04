#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class FunctionCallExpressionHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		FunctionCallExpressionHandler();
		virtual ~FunctionCallExpressionHandler();

		virtual void HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator) override;

		// Assuming we have the argument list and function address already pushed
		// onto the eval stack, generate the instructions to make the call.
		static void GenerateCallInstructions(LinkedList<Instruction*>& instructionList, const ParseParty::Lexer::FileLocation& fileLocation);
	};
}