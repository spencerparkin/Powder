#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class POWDER_API UnaryOperationConstruct : public LanguageConstruct
	{
	public:
		UnaryOperationConstruct();
		virtual ~UnaryOperationConstruct();

		virtual bool Parse(TokenList& tokenList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* operandConstruct;

		std::string* unaryOperation;
	};
}