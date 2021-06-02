#include "BinaryOperationConstruct.h"
#include "UnaryOperationConstruct.h"
#include "FunctionCallConstruct.h"
#include "AtomicConstruct.h"
#include "Exceptions.hpp"
#include <string>

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

	/*virtual*/ bool BinaryOperationConstruct::Parse(TokenList& tokenList)
	{
		while (tokenList.GetHead()->value.text == "(" && tokenList.GetTail()->value.text == ")")
		{
			tokenList.Remove(tokenList.GetHead());
			tokenList.Remove(tokenList.GetTail());
		}

		if (tokenList.GetCount() == 0)
			return false;

		struct
		{
			std::string text;
			SearchDirection searchDirection;
		} binOpArray[] = {
			// Order here indicates precedence.
			// Search direction indicates associativity.
			{"=", SearchDirection::LEFTWARD},
			{"+", SearchDirection::RIGHTWARD},
			{"-", SearchDirection::RIGHTWARD},
			{"*", SearchDirection::RIGHTWARD},
			{"/", SearchDirection::RIGHTWARD},
			{"%", SearchDirection::RIGHTWARD},
			{"^", SearchDirection::LEFTWARD},
			{"", SearchDirection::RIGHTWARD}
		};

		TokenList::Node* operatorNode = nullptr;
		for (int i = 0; binOpArray[i].text.length() > 0 && !operatorNode; i++)
			operatorNode = this->FindRootLevelToken(tokenList, binOpArray[i].text, binOpArray[i].searchDirection);

		if (!operatorNode || operatorNode == tokenList.GetHead() || operatorNode == tokenList.GetTail())
			return false;

		TokenList leftTokenList, rightTokenList;
		this->ExtractSubList(tokenList, leftTokenList, tokenList.GetHead(), operatorNode->GetPrev(), false);
		this->ExtractSubList(tokenList, rightTokenList, operatorNode->GetNext(), tokenList.GetTail(), false);

		Token operatorToken = operatorNode->value;
		tokenList.Remove(operatorNode);

		LinkedList<LanguageConstruct*> constructList;
		constructList.AddTail(new BinaryOperationConstruct());
		constructList.AddTail(new UnaryOperationConstruct());
		constructList.AddTail(new AtomicConstruct());
		constructList.AddTail(new FunctionCallConstruct());

		this->leftOperandConstruct = this->TryParseWithConstructList(leftTokenList, constructList);
		if (!leftOperandConstruct)
			throw new CompileTimeException("Failed to parse left operand of binary operator %s.", operatorToken.lineNumber);

		constructList.AddTail(new BinaryOperationConstruct());
		constructList.AddTail(new UnaryOperationConstruct());
		constructList.AddTail(new AtomicConstruct());
		constructList.AddTail(new FunctionCallConstruct());

		this->rightOperandConstruct = this->TryParseWithConstructList(rightTokenList, constructList);
		if (!this->rightOperandConstruct)
			throw new CompileTimeException("Failed to parse right operand of binary operator %s.", operatorToken.lineNumber);

		tokenList.RemoveAll();
		return true;
	}

	/*virtual*/ void BinaryOperationConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// Generate instructions for left operand, then right, then string those together.
		// At this point, we should have two results at the top of the evaluation stack.
		// We simply now tack on the appropriate math instruction which will pop those
		// results off, perform the operation, then push the result onto the stack.
	}
}