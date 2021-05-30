#include "FunctionCallConstruct.h"

namespace Powder
{
	FunctionCallConstruct::FunctionCallConstruct()
	{
	}

	/*virtual*/ FunctionCallConstruct::~FunctionCallConstruct()
	{
		DeleteList<LanguageConstruct*>(this->argumentConstructList);
	}

	/*virtual*/ LanguageConstruct* FunctionCallConstruct::New()
	{
		return new FunctionCallConstruct();
	}
	
	/*virtual*/ LanguageConstruct::ParseResult FunctionCallConstruct::Parse(TokenList& tokenList, std::list<std::string>& errorList)
	{
		return ParseResult::SYNTAX_ERROR;
	}

	/*virtual*/ void FunctionCallConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
	}
}