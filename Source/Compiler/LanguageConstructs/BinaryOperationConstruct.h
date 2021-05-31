#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	// These are what make heavy use of the evaluation stack.
	class POWDER_API BinaryOperationConstruct : public LanguageConstruct
	{
	public:
		BinaryOperationConstruct();
		virtual ~BinaryOperationConstruct();

		virtual LanguageConstruct* New() override;
		virtual ParseResult Parse(TokenList& tokenList, std::list<std::string>& errorList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* leftOperandConstruct;
		LanguageConstruct* rightOperandConstruct;

		std::string* binaryOperation;
	};
}