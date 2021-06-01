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

	/*virtual*/ bool ListConstruct::Parse(TokenList& tokenList)
	{
		return false;
	}

	/*virtual*/ void ListConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
	}
}