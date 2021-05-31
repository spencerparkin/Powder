#pragma once

#include "LanguageConstruct.h"
#include <list>

namespace Powder
{
	class POWDER_API FunctionConstruct : public LanguageConstruct
	{
	public:
		FunctionConstruct();
		virtual ~FunctionConstruct();

		virtual LanguageConstruct* New() override;
		virtual ParseResult Parse(TokenList& tokenList, std::list<std::string>& errorList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* bodyConstruct;

		std::list<std::string>* namedArgumentsList;
	};
}