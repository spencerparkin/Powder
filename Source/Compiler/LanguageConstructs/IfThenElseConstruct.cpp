#include "IfThenElseConstruct.h"
#include "Tokenizer.h"
#include "Exceptions.hpp"
#include "BinaryOperationConstruct.h"
#include "UnaryOperationConstruct.h"
#include "FunctionCallConstruct.h"
#include "ProgramConstruct.h"

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

	/*virtual*/ bool IfThenElseConstruct::Parse(TokenList& tokenList)
	{
		if (!tokenList.GetHead())
			return false;

		Token ifKeywordToken = tokenList.GetHead()->value;
		if (ifKeywordToken.text != "if")
			return false;

		if (!tokenList.GetHead()->GetNext())
			throw new CompileTimeException("No tokens after \"if\" keyword.", ifKeywordToken.lineNumber);

		TokenList::Node* conditionOpener = tokenList.GetHead()->GetNext();
		if (conditionOpener->value.text != "(")
			throw new CompileTimeException("Expected open paran after \"if\" function kewyord.", ifKeywordToken.lineNumber);

		TokenList::Node* conditionCloser = this->FindCloserMatchingOpener(conditionOpener);
		if (!conditionCloser)
			throw new CompileTimeException("Did not find matching close paran for if-statement's open paran as part of the conditional.", conditionOpener->value.lineNumber);

		TokenList::Node* conditionPassOpener = conditionCloser->GetNext();
		if (!conditionPassOpener)
			throw new CompileTimeException("No tokens after if-statement conditional.", conditionCloser->value.lineNumber);

		if (conditionPassOpener->value.text != "{")
			throw new CompileTimeException("Expected curly brace as opener for condition-pass block of if-statement.", conditionPassOpener->value.lineNumber);

		TokenList::Node* conditionPassCloser = this->FindCloserMatchingOpener(conditionPassOpener);
		if (!conditionPassCloser)
			throw new CompileTimeException("Did not find matching closing curly brace for condition-pass block of if-statement.", conditionPassOpener->value.lineNumber);

		TokenList conditionFailTokenList;
		Token elseKeywordToken;
		if (conditionPassCloser->GetNext() && conditionPassCloser->GetNext()->value.text == "else")
		{
			elseKeywordToken = conditionPassCloser->GetNext()->value;
			if (!conditionPassCloser->GetNext()->GetNext())
				throw new CompileTimeException("No tokens after if-statement's else clause.", elseKeywordToken.lineNumber);

			TokenList::Node* conditionFailOpener = conditionPassCloser->GetNext()->GetNext();
			if (conditionFailOpener->value.text != "{")
				throw new CompileTimeException("Expected curly brace as opener for condition-fail block of if-statment.", conditionFailOpener->value.lineNumber);

			TokenList::Node* conditionFailCloser = this->FindCloserMatchingOpener(conditionFailOpener);
			if (!conditionFailCloser)
				throw new CompileTimeException("Did not ifnd matching closing curly brace for condition-fail block of if-statment.", conditionFailOpener->value.lineNumber);

			tokenList.Remove(conditionPassCloser->GetNext());
			this->ExtractSubList(tokenList, conditionFailTokenList, conditionFailOpener, conditionFailCloser, true);
		}

		TokenList conditionPassTokenList;
		this->ExtractSubList(tokenList, conditionPassTokenList, conditionPassOpener, conditionPassCloser, true);

		TokenList conditionTokenList;
		this->ExtractSubList(tokenList, conditionTokenList, conditionOpener, conditionCloser, true);

		tokenList.Remove(tokenList.GetHead());

		LinkedList<LanguageConstruct*> constructList;
		constructList.AddTail(new BinaryOperationConstruct());
		constructList.AddTail(new UnaryOperationConstruct());
		constructList.AddTail(new FunctionCallConstruct());

		this->conditionalConstruct = this->TryParseWithConstructList(conditionTokenList, constructList);
		if (!this->conditionalConstruct)
			throw new CompileTimeException("Failed to parse if-statement conditional.", ifKeywordToken.lineNumber);

		this->conditionPassConstruct = new ProgramConstruct();
		if (!this->conditionPassConstruct->Parse(conditionPassTokenList))
			throw new CompileTimeException("Failed to parse if-statment condition pass block.", ifKeywordToken.lineNumber);

		this->conditionFailConstruct = new ProgramConstruct();
		if (!this->conditionFailConstruct->Parse(conditionFailTokenList))
			throw new CompileTimeException("Failed to parse if-statment condition fail block.", elseKeywordToken.lineNumber);

		return true;
	}

	/*virtual*/ void IfThenElseConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// TODO: Use the branch instruction.  Remember to add an unconditional jump if needed.
	}
}