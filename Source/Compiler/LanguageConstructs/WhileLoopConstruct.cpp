#include "WhileLoopConstruct.h"

namespace Powder
{
	WhileLoopConstruct::WhileLoopConstruct()
	{
		this->conditionalConstruct = nullptr;
		this->bodyConstruct = nullptr;
	}

	/*virtual*/ WhileLoopConstruct::~WhileLoopConstruct()
	{
		delete this->conditionalConstruct;
		delete this->bodyConstruct;
	}

	/*virtual*/ LanguageConstruct* WhileLoopConstruct::New()
	{
		return new WhileLoopConstruct();
	}

	/*virtual*/ LanguageConstruct::ParseResult WhileLoopConstruct::Parse(TokenList& tokenList, std::list<std::string>& errorList)
	{
		return ParseResult::SYNTAX_ERROR;
	}

	/*virtual*/ void WhileLoopConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
	}
}