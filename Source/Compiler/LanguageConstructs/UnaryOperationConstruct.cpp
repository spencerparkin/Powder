#include "UnaryOperationConstruct.h"

namespace Powder
{
	UnaryOperationConstruct::UnaryOperationConstruct()
	{
		this->operandConstruct = nullptr;
	}

	/*virtual*/ UnaryOperationConstruct::~UnaryOperationConstruct()
	{
		delete this->operandConstruct;
	}

	/*virtual*/ LanguageConstruct* UnaryOperationConstruct::New()
	{
		return new UnaryOperationConstruct();
	}

	/*virtual*/ LanguageConstruct::ParseResult UnaryOperationConstruct::Parse(TokenList& tokenList, std::list<std::string>& errorList)
	{
		// See if we're a left or right unary operator.  If we are,
		// try to parse the remainder as another expression for a function
		// call or something.  Note that we must be sensative of binding
		// here.  For example, don't parse a + b~ as (a + b)~.

		return ParseResult::SYNTAX_ERROR;
	}

	/*virtual*/ void UnaryOperationConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		//...
	}
}