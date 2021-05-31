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
		virtual ParseResult Parse(TokenList& tokenList, std::list<std::string>& errorList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:

		std::string operandText;
	};
}