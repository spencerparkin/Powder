#include "FunctionDefinitionConstruct.h"
#include "ProgramConstruct.h"
#include "Exceptions.hpp"

namespace Powder
{
	FunctionDefinitionConstruct::FunctionDefinitionConstruct()
	{
		this->bodyConstruct = new ProgramConstruct();
		this->functionName = new std::string;
		this->namedArgumentsList = new std::list<std::string>;
	}

	/*virtual*/ FunctionDefinitionConstruct::~FunctionDefinitionConstruct()
	{
		delete this->bodyConstruct;
		delete this->functionName;
		delete this->namedArgumentsList;
	}

	/*virtual*/ LanguageConstruct* FunctionDefinitionConstruct::New()
	{
		return new FunctionDefinitionConstruct();
	}

	/*virtual*/ bool FunctionDefinitionConstruct::Parse(TokenList& tokenList)
	{
		if (!tokenList.GetHead())
			return false;

		Token fundKeywordToken = tokenList.GetHead()->value;
		if (fundKeywordToken.text != "func")
			return false;

		if (!tokenList.GetHead()->GetNext())
			throw new CompileTimeException("No tokens after \"func\" keyword.", fundKeywordToken.lineNumber);
		
		Token functionIdentifierToken = tokenList.GetHead()->GetNext()->value;
		if (functionIdentifierToken.type != Token::IDENTIFIER)
			throw new CompileTimeException("Expected identifier after \"func\" keyword.", functionIdentifierToken.lineNumber);

		*this->functionName = functionIdentifierToken.text;

		if (!tokenList.GetHead()->GetNext()->GetNext())
			throw new CompileTimeException("No tokens after function identifier.", functionIdentifierToken.lineNumber);

		TokenList::Node* argListOpener = tokenList.GetHead()->GetNext()->GetNext();
		if (argListOpener->value.text != "(")
			throw new CompileTimeException("Expected open paran after function identifier.", argListOpener->value.lineNumber);
		
		TokenList::Node* argListCloser = this->FindCloserMatchingOpener(argListOpener);
		if (!argListCloser)
			throw new CompileTimeException("Did not find matching close paran for function definition's parameter list open paran.", argListOpener->value.lineNumber);

		if (!argListCloser->GetNext())
			throw new CompileTimeException("No tokens found after function definition's parameter list close paran.", argListCloser->value.lineNumber);

		TokenList::Node* bodyOpener = argListCloser->GetNext();
		if (bodyOpener->value.text != "{")
			throw new CompileTimeException("Expected openc curly bracket after function definition's parameter list close paran.", bodyOpener->value.lineNumber);
		
		TokenList::Node* bodyCloser = this->FindCloserMatchingOpener(bodyOpener);
		if (!bodyCloser)
			throw new CompileTimeException("Did not find matching closing curly brace for function definition's body.", bodyOpener->value.lineNumber);

		TokenList argTokenList;
		this->ExtractSubList(tokenList, argTokenList, argListOpener, argListCloser);

		TokenList bodyTokenList;
		this->ExtractSubList(tokenList, bodyTokenList, bodyOpener, bodyCloser);

		tokenList.Remove(tokenList.GetHead());
		tokenList.Remove(tokenList.GetHead());

		argTokenList.Remove(argTokenList.GetHead());
		argTokenList.Remove(argTokenList.GetTail());

		while (argTokenList.GetCount() > 0)
		{
			Token argToken = argTokenList.GetHead()->value;
			if (argToken.type != Token::IDENTIFIER)
				throw new CompileTimeException("Expected to find identifier for argument in function definition.", argToken.lineNumber);

			this->namedArgumentsList->push_back(argToken.text);
			argTokenList.Remove(argTokenList.GetHead());

			if (argTokenList.GetCount() > 0)
			{
				Token commaToken = argTokenList.GetHead()->value;
				if (commaToken.text != ",")
					throw new CompileTimeException("Expected to find comman delimeter separating named argument identifiers.", commaToken.lineNumber);

				argTokenList.Remove(argTokenList.GetHead());
			}
		}

		bodyTokenList.Remove(bodyTokenList.GetHead());
		bodyTokenList.Remove(bodyTokenList.GetTail());

		if (!this->bodyConstruct->Parse(bodyTokenList))
			throw new CompileTimeException("Failed to parse function definition's body as program construct.", fundKeywordToken.lineNumber);

		return true;
	}

	/*virtual*/ void FunctionDefinitionConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
	}
}