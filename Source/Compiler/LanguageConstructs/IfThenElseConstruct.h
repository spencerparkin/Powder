#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class IfThenElseConstruct : public LanguageConstruct
	{
	public:
		IfThenElseConstruct();
		virtual ~IfThenElseConstruct();

		virtual LanguageConstruct* New() override;
		virtual ParseResult Parse(TokenList& tokenList, std::list<std::string>& errorList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* conditionalConstruct;
		LanguageConstruct* conditionPassConstruct;
		LanguageConstruct* conditionFailConstruct;
	};
}