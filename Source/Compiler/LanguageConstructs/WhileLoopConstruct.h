#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class POWDER_API WhileLoopConstruct : public LanguageConstruct
	{
	public:
		WhileLoopConstruct();
		virtual ~WhileLoopConstruct();

		virtual LanguageConstruct* New() override;
		virtual bool Parse(TokenList& tokenList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* conditionalConstruct;
		LanguageConstruct* bodyConstruct;
	};
}