#include "FunctionCallConstruct.h"

namespace Powder
{
	FunctionCallConstruct::FunctionCallConstruct()
	{
		this->functionName = new std::string;
	}

	/*virtual*/ FunctionCallConstruct::~FunctionCallConstruct()
	{
		DeleteList<LanguageConstruct*>(this->argumentConstructList);
		delete this->functionName;
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