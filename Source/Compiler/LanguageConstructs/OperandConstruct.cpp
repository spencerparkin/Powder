#include "OperandConstruct.h"

namespace Powder
{
	OperandConstruct::OperandConstruct()
	{
		this->operandText = new std::string;
	}

	/*virtual*/ OperandConstruct::~OperandConstruct()
	{
		delete this->operandText;
	}

	/*virtual*/ LanguageConstruct* OperandConstruct::New()
	{
		return new OperandConstruct();
	}

	/*virtual*/ bool OperandConstruct::Parse(TokenList& tokenList)
	{
		return false;
	}

	/*virtual*/ void OperandConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// This will typically just be a single push instruction to create a
		// new immediate value on the evaluation stack.  But it could also
		// be a load instruction to get a value from scope and then put that
		// on the evaluation stack.
	}
}