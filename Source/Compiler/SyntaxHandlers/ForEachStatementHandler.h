#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ForEachStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ForEachStatementHandler();
		virtual ~ForEachStatementHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;

		static bool GenerateInstructionForIteratorCallSetup(LinkedList<Instruction*>& instructionList, const char* action, const ParseParty::Lexer::FileLocation& fileLocation, Error& error);
	};
}