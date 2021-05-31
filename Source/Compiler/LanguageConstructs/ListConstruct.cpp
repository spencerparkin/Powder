#include "ListConstruct.h"
#include "LinkedList.hpp"

namespace Powder
{
	ListConstruct::ListConstruct()
	{
	}

	/*virtual*/ ListConstruct::~ListConstruct()
	{
		DeleteList<LanguageConstruct*>(this->elementConstructList);
	}

	/*virtual*/ LanguageConstruct* ListConstruct::New()
	{
		return new ListConstruct();
	}

	/*virtual*/ LanguageConstruct::ParseResult ListConstruct::Parse(TokenList& tokenList, std::list<std::string>& errorList)
	{
		return ParseResult::SYNTAX_ERROR;
	}

	/*virtual*/ void ListConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
	}
}