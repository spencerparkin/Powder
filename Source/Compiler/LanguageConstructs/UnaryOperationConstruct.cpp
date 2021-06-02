#include "UnaryOperationConstruct.h"
#include "BinaryOperationConstruct.h"
#include "AtomicConstruct.h"
#include "FunctionCallConstruct.h"
#include "Tokenizer.h"
#include "Exceptions.hpp"

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

	/*virtual*/ bool UnaryOperationConstruct::Parse(TokenList& tokenList)
	{
		TokenList tokenListCopy;
		tokenListCopy.Append(tokenList);

		while (tokenListCopy.GetHead()->value.text == "(" && tokenListCopy.GetTail()->value.text == ")")
		{
			tokenListCopy.Remove(tokenListCopy.GetHead());
			tokenListCopy.Remove(tokenListCopy.GetTail());
		}

		if (tokenListCopy.GetCount() < 2)
			return false;

		struct
		{
			std::string text;
			bool onLeft;
		} uniOpArray[] = {
			{"~", true},
			{"!", true},
			{"", false}
		};

		TokenList::Node* operatorNode = nullptr;
		int i;
		for (i = 0; uniOpArray[i].text.length() > 0 && !operatorNode; i++)
		{
			if (uniOpArray[i].onLeft)
			{
				if (tokenListCopy.GetHead()->value.text == uniOpArray[i].text)
					operatorNode = tokenListCopy.GetHead();
			}
			else
			{
				if (tokenListCopy.GetTail()->value.text == uniOpArray[i].text)
					operatorNode = tokenListCopy.GetTail();
			}
		}

		if (!operatorNode)
			return false;

		Token operatorToken = operatorNode->value;
		tokenListCopy.Remove(operatorNode);
		
		LinkedList<LanguageConstruct*> constructList;
		constructList.AddTail(new AtomicConstruct());
		constructList.AddTail(new FunctionCallConstruct());
		constructList.AddTail(new UnaryOperationConstruct());

		this->operandConstruct = this->TryParseWithConstructList(tokenList, constructList);
		if (!this->operandConstruct)
		{
			if ((uniOpArray[i].onLeft && tokenListCopy.GetHead()->value.type == Token::OPENER) ||
				(!uniOpArray[i].onLeft && tokenListCopy.GetTail()->value.type == Token::CLOSER))
			{
				constructList.AddTail(new BinaryOperationConstruct());
				this->operandConstruct = this->TryParseWithConstructList(tokenList, constructList);
				if (!this->operandConstruct)
				{
					throw new CompileTimeException("Failed to parse operand of unary operator %s.", operatorToken.lineNumber);
				}
			}
		}

		tokenList.RemoveAll();
		return true;
	}

	/*virtual*/ void UnaryOperationConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		//...
	}
}