#include "BinaryOperationConstruct.h"

namespace Powder
{
	BinaryOperationConstruct::BinaryOperationConstruct()
	{
		this->leftOperandConstruct = nullptr;
		this->rightOperandConstruct = nullptr;
	}

	/*virtual*/ BinaryOperationConstruct::~BinaryOperationConstruct()
	{
		delete this->leftOperandConstruct;
		delete this->rightOperandConstruct;
	}

	/*virtual*/ LanguageConstruct* BinaryOperationConstruct::New()
	{
		return new BinaryOperationConstruct();
	}

	/*virtual*/ LanguageConstruct::ParseResult BinaryOperationConstruct::Parse(TokenList& tokenList, std::list<std::string>& errorList)
	{
		return ParseResult::SYNTAX_ERROR;
	}

	/*virtual*/ void BinaryOperationConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// Generate instructions for left operand, then right, then string those together.
		// At this point, we should have two results at the top of the evaluation stack.
		// We simply now tack on the appropriate math instruction which will pop those
		// results off, perform the operatoin, then push the result onto the stack.
	}
}