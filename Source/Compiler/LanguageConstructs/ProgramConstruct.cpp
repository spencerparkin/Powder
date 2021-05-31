#include "ProgramConstruct.h"
#include "BinaryOperationConstruct.h"
#include "UnaryOperationConstruct.h"
#include "FunctionConstruct.h"
#include "IfThenElseConstruct.h"
#include "WhileLoopConstruct.h"

namespace Powder
{
	ProgramConstruct::ProgramConstruct()
	{
	}

	/*virtual*/ ProgramConstruct::~ProgramConstruct()
	{
		DeleteList<LanguageConstruct*>(this->constructList);
	}

	/*virtual*/ LanguageConstruct* ProgramConstruct::New()
	{
		return new ProgramConstruct();
	}

	/*virtual*/ LanguageConstruct::ParseResult ProgramConstruct::Parse(TokenList& tokenList, std::list<std::string>& errorList)
	{
		LinkedList<LanguageConstruct*> languageConstructList;
		languageConstructList.AddTail(new FunctionConstruct());
		languageConstructList.AddTail(new IfThenElseConstruct());
		languageConstructList.AddTail(new WhileLoopConstruct());
		languageConstructList.AddTail(new BinaryOperationConstruct());
		languageConstructList.AddTail(new UnaryOperationConstruct());

		while (tokenList.GetCount() > 0)
		{
			uint32_t listSize = this->constructList.GetCount();

			for (LinkedList<LanguageConstruct*>::Node* node = languageConstructList.GetHead(); node; node = node->GetNext())
			{
				LanguageConstruct* languageConstruct = node->value;
				ParseResult parseResult = languageConstruct->Parse(tokenList, errorList);
				if (parseResult == ParseResult::SUCCESS)
				{
					this->constructList.AddTail(languageConstruct);
					node->value = languageConstruct->New();
					break;
				}
				else if (parseResult == ParseResult::SYNTAX_ERROR)
					break;
			}

			if(listSize == this->constructList.GetCount())
			{
				errorList.push_back("Could not parse code starting on line %d as any recognizeable program construct.");
				return ParseResult::SYNTAX_ERROR;
			}
		}

		return ParseResult::SUCCESS;
	}

	/*virtual*/ void ProgramConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// TODO: Sort all function constructs to the bottom of our list, and make sure
		//       we do a halt instruction before the program hits the first function.
	}
}