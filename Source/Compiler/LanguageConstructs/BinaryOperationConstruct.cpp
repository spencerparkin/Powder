#include "BinaryOperationConstruct.h"

namespace Powder
{
	BinaryOperationConstruct::BinaryOperationConstruct()
	{
		this->leftOperandConstruct = nullptr;
		this->rightOperandConstruct = nullptr;
		this->binaryOperation = new std::string;
	}

	/*virtual*/ BinaryOperationConstruct::~BinaryOperationConstruct()
	{
		delete this->leftOperandConstruct;
		delete this->rightOperandConstruct;
		delete this->binaryOperation;
	}

	/*virtual*/ LanguageConstruct* BinaryOperationConstruct::New()
	{
		return new BinaryOperationConstruct();
	}

	/*virtual*/ bool BinaryOperationConstruct::Parse(TokenList& tokenList)
	{


		return false;
	}

	/*virtual*/ void BinaryOperationConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// Generate instructions for left operand, then right, then string those together.
		// At this point, we should have two results at the top of the evaluation stack.
		// We simply now tack on the appropriate math instruction which will pop those
		// results off, perform the operation, then push the result onto the stack.
	}
}