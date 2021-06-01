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
	
	/*virtual*/ bool FunctionCallConstruct::Parse(TokenList& tokenList)
	{
		return false;
	}

	/*virtual*/ void FunctionCallConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// Note that this could also be a system call.
	}
}