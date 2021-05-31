#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class POWDER_API UnaryOperationConstruct : public LanguageConstruct
	{
	public:
		UnaryOperationConstruct();
		virtual ~UnaryOperationConstruct();

		virtual LanguageConstruct* New() override;
		virtual ParseResult Parse(TokenList& tokenList, std::list<std::string>& errorList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* operandConstruct;

		std::string* unaryOperation;
	};
}