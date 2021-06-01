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

	/*virtual*/ LanguageConstruct* ProgramConstruct::New()
	{
		return new ProgramConstruct();
	}

	/*virtual*/ bool ProgramConstruct::Parse(TokenList& tokenList)
	{
		LinkedList<LanguageConstruct*> languageConstructList;

		// Note that the order here is significant and intentional.
		// Most constructs are easily recognizable by their use of keywords.
		// When all such constructs are exhausted, what remains is an attempt
		// to parse as a binary expression.
		languageConstructList.AddTail(new FunctionDefinitionConstruct());
		languageConstructList.AddTail(new FunctionCallConstruct());
		languageConstructList.AddTail(new IfThenElseConstruct());
		languageConstructList.AddTail(new WhileLoopConstruct());
		languageConstructList.AddTail(new BinaryOperationConstruct());

		while (tokenList.GetCount() > 0)
		{
			uint32_t listSize = this->constructList.GetCount();

			for (LinkedList<LanguageConstruct*>::Node* node = languageConstructList.GetHead(); node; node = node->GetNext())
			{
				LanguageConstruct* languageConstruct = node->value;
				if (languageConstruct->Parse(tokenList))
				{
					this->constructList.AddTail(languageConstruct);
					node->value = languageConstruct->New();
					break;
				}
			}

			if(listSize == this->constructList.GetCount())
				throw new CompileTimeException("Could not parse code as any recognizeable program construct.", tokenList.GetHead()->value.lineNumber);
		}

		DeleteList<LanguageConstruct*>(languageConstructList);
		return true;
	}

	/*virtual*/ void ProgramConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// TODO: Sort all function constructs to the bottom of our list, and make sure
		//       we do a halt instruction before the program hits the first function.
	}
}