#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class POWDER_API BinaryOperationConstruct : public LanguageConstruct
	{
	public:
		BinaryOperationConstruct();
		virtual ~BinaryOperationConstruct();

		virtual bool Parse(TokenList& tokenList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* leftOperandConstruct;
		LanguageConstruct* rightOperandConstruct;

		std::string* binaryOperation;
	};
}