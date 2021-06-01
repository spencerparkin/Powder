#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class POWDER_API IfThenElseConstruct : public LanguageConstruct
	{
	public:
		IfThenElseConstruct();
		virtual ~IfThenElseConstruct();

		virtual LanguageConstruct* New() override;
		virtual bool Parse(TokenList& tokenList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* conditionalConstruct;
		LanguageConstruct* conditionPassConstruct;
		LanguageConstruct* conditionFailConstruct;
	};
}