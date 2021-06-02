#pragma once

#include "Defines.h"
#include "LinkedList.hpp"
#include "Tokenizer.h"
#include <string>

namespace Powder
{
	class Instruction;

	// Ultimately what we want is to build a tree of language constructs as
	// a function of a given token list.  If this succeeds, then the token
	// list is grammatically correct, and therefore, represents a valid program,
	// even if the program doesn't necessarily do what the programmer intends.
	// Note that the use of a grammar-engine to produce an abstract-syntax-tree is
	// probably a much better approach to this problem, but it is also much more
	// complicated.  The language syntax is easier to maintain if it is described
	// by a set of grammar rules.  That withstanding, this approach is simpler
	// and accomplishes the same goal, even if it is less flexible or easier to maintain.
	class POWDER_API LanguageConstruct
	{
	public:
		LanguageConstruct();
		virtual ~LanguageConstruct();

		virtual bool Parse(TokenList& tokenList) = 0;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) = 0;

	protected:

		LanguageConstruct* TryParseWithConstructList(TokenList& tokenList, LinkedList<LanguageConstruct*>& constructList);

		enum SearchDirection
		{
			RIGHTWARD,
			LEFTWARD
		};

		TokenList::Node* FindRootLevelToken(TokenList& tokenList, const std::string& text, SearchDirection searchDirection);
		
		TokenList::Node* FindCloserMatchingOpener(TokenList::Node* node);

		void ExtractSubList(TokenList& parentList, TokenList& childList, TokenList::Node* leftNode, TokenList::Node* rightNode, bool trimBothEnds);
	};
}