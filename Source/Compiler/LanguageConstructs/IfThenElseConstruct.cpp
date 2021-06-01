#include "IfThenElseConstruct.h"

namespace Powder
{
	IfThenElseConstruct::IfThenElseConstruct()
	{
		this->conditionalConstruct = nullptr;
		this->conditionPassConstruct = nullptr;
		this->conditionFailConstruct = nullptr;
	}

	/*virtual*/ IfThenElseConstruct::~IfThenElseConstruct()
	{
		delete this->conditionalConstruct;
		delete this->conditionPassConstruct;
		delete this->conditionFailConstruct;
	}

	/*virtual*/ LanguageConstruct* IfThenElseConstruct::New()
	{
		return new IfThenElseConstruct();
	}

	/*virtual*/ bool IfThenElseConstruct::Parse(TokenList& tokenList)
	{
		return false;
	}

	/*virtual*/ void IfThenElseConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// TODO: Use the branch instruction.  Remember to add an unconditional jump if needed.
	}
}