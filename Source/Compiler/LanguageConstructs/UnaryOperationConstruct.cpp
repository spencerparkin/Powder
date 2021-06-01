#include "UnaryOperationConstruct.h"

namespace Powder
{
	UnaryOperationConstruct::UnaryOperationConstruct()
	{
		this->operandConstruct = nullptr;
		this->unaryOperation = new std::string;
	}

	/*virtual*/ UnaryOperationConstruct::~UnaryOperationConstruct()
	{
		delete this->operandConstruct;
		delete this->unaryOperation;
	}

	/*virtual*/ LanguageConstruct* UnaryOperationConstruct::New()
	{
		return new UnaryOperationConstruct();
	}

	/*virtual*/ bool UnaryOperationConstruct::Parse(TokenList& tokenList)
	{
		// See if we're a left or right unary operator.  If we are,
		// try to parse the remainder as another expression for a function
		// call or something.  Note that we must be sensative of binding
		// here.  For example, don't parse a + b~ as (a + b)~.

		return false;
	}

	/*virtual*/ void UnaryOperationConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		//...
	}
}