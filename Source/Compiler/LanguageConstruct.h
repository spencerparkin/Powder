#pragma once

#include "Defines.h"
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
	class POWDER_API LanguageConstruct
	{
	public:
		LanguageConstruct();
		virtual ~LanguageConstruct();

		virtual LanguageConstruct* New() = 0;
		virtual bool Parse(TokenList& tokenList) = 0;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) = 0;

	protected:

		TokenList::Node* FindCloserMatchingOpener(TokenList::Node* node);
		void ExtractSubList(TokenList& parentList, TokenList& childList, TokenList::Node* leftNode, TokenList::Node* rightNode);
	};
}