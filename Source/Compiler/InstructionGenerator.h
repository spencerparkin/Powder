#pragma once

#include "LinkedList.hpp"
#include "Parser.h"

namespace Powder
{
	class Instruction;

	class InstructionGenerator
	{
	public:
		InstructionGenerator();
		virtual ~InstructionGenerator();

		void GenerateInstructionList(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode);
	};
}