#include "FunctionCallConstruct.h"
#include "BinaryOperationConstruct.h"
#include "UnaryOperationConstruct.h"
#include "AtomicConstruct.h"
#include "Tokenizer.h"
#include "Exceptions.hpp"

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
	
	/*virtual*/ bool FunctionCallConstruct::Parse(TokenList& tokenList)
	{
		if (!tokenList.GetHead())
			return false;

		Token funcNameToken = tokenList.GetHead()->value;
		if (funcNameToken.type != Token::IDENTIFIER)
			return false;

		TokenList::Node* argListOpener = tokenList.GetHead()->GetNext();
		if (!argListOpener)
			return false;

		if (argListOpener->value.text != "(")
			return false;

		TokenList::Node* argListCloser = this->FindCloserMatchingOpener(argListOpener);
		if(!argListCloser)
			throw new CompileTimeException("Function %s call argument list open paran is without a maching close paran.", argListOpener->value.lineNumber);

		*this->functionName = funcNameToken.text;

		tokenList.Remove(tokenList.GetHead());

		TokenList argumentsTokenList;
		this->ExtractSubList(tokenList, argumentsTokenList, argListOpener, argListCloser, true);

		while (true)
		{
			TokenList argTokenList;

			TokenList::Node* node = this->FindRootLevelToken(argumentsTokenList, ",", SearchDirection::RIGHTWARD);
			if (!node)
				argTokenList.Append(argumentsTokenList);
			else
			{
				this->ExtractSubList(argumentsTokenList, argTokenList, argumentsTokenList.GetHead(), node->GetPrev(), false);
				argumentsTokenList.Remove(node);
			}

			LinkedList<LanguageConstruct*> constructList;
			constructList.AddTail(new FunctionCallConstruct());
			constructList.AddTail(new BinaryOperationConstruct());
			constructList.AddTail(new UnaryOperationConstruct());
			constructList.AddTail(new AtomicConstruct());

			LanguageConstruct* argumentConstruct = this->TryParseWithConstructList(argTokenList, constructList);
			if (!argumentConstruct)
				throw new CompileTimeException("Failed to parse argument %d of function %s.", argTokenList.GetHead()->value.lineNumber);

			this->argumentConstructList.AddTail(argumentConstruct);
		}

		return true;
	}

	/*virtual*/ void FunctionCallConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// Note that this could also be a system call.
	}
}