#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	struct Token;

	class POWDER_API AtomicConstruct : public LanguageConstruct
	{
	public:
		AtomicConstruct();
		virtual ~AtomicConstruct();

		virtual bool Parse(TokenList& tokenList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:

		Token* token;
	};
}