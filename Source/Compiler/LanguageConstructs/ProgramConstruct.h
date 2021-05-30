#pragma once

#include "LanguageConstruct.h"
#include "LinkedList.hpp"

namespace Powder
{
	// This is the most fundamental of all language constructs.
	// It is nothing more than a list of language constructs,
	// each to be executed in the order given.  This must be
	// the root of every syntax tree, and therefore, we can define
	// a validly written program as one that successfully parses
	// as a program construct.  Note that it also often appears as the
	// child or children of all other constructs.
	class ProgramConstruct : public LanguageConstruct
	{
	public:
		ProgramConstruct();
		virtual ~ProgramConstruct();

		virtual LanguageConstruct* New() override;
		virtual ParseResult Parse(TokenList& tokenList, std::list<std::string>& errorList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LinkedList<LanguageConstruct*> constructList;
	};
}