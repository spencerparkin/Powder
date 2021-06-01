#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class POWDER_API OperandConstruct : public LanguageConstruct
	{
	public:
		OperandConstruct();
		virtual ~OperandConstruct();

		virtual LanguageConstruct* New() override;
		virtual bool Parse(TokenList& tokenList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:

		std::string* operandText;
	};
}