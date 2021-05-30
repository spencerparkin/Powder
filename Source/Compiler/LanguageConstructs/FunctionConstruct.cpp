#include "FunctionConstruct.h"
#include "ProgramConstruct.h"

namespace Powder
{
	FunctionConstruct::FunctionConstruct()
	{
		this->bodyConstruct = new ProgramConstruct();
	}

	/*virtual*/ FunctionConstruct::~FunctionConstruct()
	{
		delete this->bodyConstruct;
	}

	/*virtual*/ LanguageConstruct* FunctionConstruct::New()
	{
		return new FunctionConstruct();
	}

	/*virtual*/ LanguageConstruct::ParseResult FunctionConstruct::Parse(TokenList& tokenList, std::list<std::string>& errorList)
	{
		ParseResult parseResult = this->bodyConstruct->Parse(tokenList, errorList);
		if(parseResult != ParseResult::SUCCESS)
		{
			errorList.push_back("Function body starting at line %d did not parse as program construct.");
			return ParseResult::SYNTAX_ERROR;
		}

		return ParseResult::SUCCESS;
	}

	/*virtual*/ void FunctionConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
	}
}