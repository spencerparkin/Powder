#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class POWDER_API ListConstruct : public LanguageConstruct
	{
	public:
		ListConstruct();
		virtual ~ListConstruct();

		virtual LanguageConstruct* New() override;
		virtual ParseResult Parse(TokenList& tokenList, std::list<std::string>& errorList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:

		LinkedList<LanguageConstruct*> elementConstructList;
	};
}