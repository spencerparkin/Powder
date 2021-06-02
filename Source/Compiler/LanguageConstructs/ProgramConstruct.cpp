#include "ProgramConstruct.h"
#include "BinaryOperationConstruct.h"
#include "FunctionDefinitionConstruct.h"
#include "FunctionCallConstruct.h"
#include "IfThenElseConstruct.h"
#include "WhileLoopConstruct.h"
#include "Exceptions.hpp"

namespace Powder
{
	ProgramConstruct::ProgramConstruct()
	{
	}

	/*virtual*/ ProgramConstruct::~ProgramConstruct()
	{
		DeleteList<LanguageConstruct*>(this->constructList);
	}

	/*virtual*/ bool ProgramConstruct::Parse(TokenList& tokenList)
	{
		while(tokenList.GetCount() > 0)
		{
			LinkedList<LanguageConstruct*> languageConstructList;
			languageConstructList.AddTail(new FunctionDefinitionConstruct());
			languageConstructList.AddTail(new FunctionCallConstruct());
			languageConstructList.AddTail(new IfThenElseConstruct());
			languageConstructList.AddTail(new WhileLoopConstruct());
			languageConstructList.AddTail(new BinaryOperationConstruct());

			LanguageConstruct* languageConstruct = this->TryParseWithConstructList(tokenList, languageConstructList);
			if (!languageConstruct)
				throw new CompileTimeException("No recognizable program construct could be parsed.", tokenList.GetHead()->value.lineNumber);

			this->constructList.AddTail(languageConstruct);
		}

		return true;
	}

	/*virtual*/ void ProgramConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// TODO: Sort all function constructs to the bottom of our list, and make sure
		//       we do a halt instruction before the program hits the first function.
	}
}