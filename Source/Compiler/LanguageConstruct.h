#pragma once

#include "LinkedList.hpp"
#include "Tokenizer.h"
#include <list>
#include <string>

namespace Powder
{
	class Instruction;

	// Ultimately what we want is to build a tree of language constructs as
	// a function of a given token list.  If this succeeds, then the token
	// list is grammatically correct, and therefore, represents a valid program,
	// even if the program doesn't necessarily do what the programmer intends.
	class LanguageConstruct
	{
	public:
		LanguageConstruct();
		virtual ~LanguageConstruct();

		virtual LanguageConstruct* New() = 0;

		enum ParseResult
		{
			NOT_RECOGNIZED,
			SYNTAX_ERROR,
			SUCCESS
		};

		virtual ParseResult Parse(TokenList& tokenList, std::list<std::string>& errorList) = 0;

		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) = 0;
	};
}