#pragma once

#include "InstructionGenerator.h"

namespace Powder
{
	class ForLoopStatementHandler : public InstructionGenerator::SyntaxHandler
	{
	public:
		ForLoopStatementHandler();
		virtual ~ForLoopStatementHandler();

		virtual bool HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error) override;

		bool GenerateInstructionForIteratorCallSetup(LinkedList<Instruction*>& instructionList, const char* action, const ParseParty::Lexer::FileLocation& fileLocation, Error& error);
	};
}