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

	/*virtual*/ bool WhileLoopConstruct::Parse(TokenList& tokenList)
	{
		return false;
	}

	/*virtual*/ void WhileLoopConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
	}
}