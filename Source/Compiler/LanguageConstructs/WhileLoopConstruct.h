#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class WhileLoopConstruct : public LanguageConstruct
	{
	public:
		WhileLoopConstruct();
		virtual ~WhileLoopConstruct();

		virtual LanguageConstruct* New() override;
		virtual ParseResult Parse(TokenList& tokenList, std::list<std::string>& errorList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* conditionalConstruct;
		LanguageConstruct* bodyConstruct;
	};
}