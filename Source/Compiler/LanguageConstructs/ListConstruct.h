#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class POWDER_API ListConstruct : public LanguageConstruct
	{
	public:
		ListConstruct();
		virtual ~ListConstruct();

		virtual bool Parse(TokenList& tokenList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:

		LinkedList<LanguageConstruct*> elementConstructList;
	};
}