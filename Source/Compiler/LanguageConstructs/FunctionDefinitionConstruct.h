#pragma once

#include "LanguageConstruct.h"
#include <list>

namespace Powder
{
	class POWDER_API FunctionDefinitionConstruct : public LanguageConstruct
	{
	public:
		FunctionDefinitionConstruct();
		virtual ~FunctionDefinitionConstruct();

		virtual LanguageConstruct* New() override;
		virtual bool Parse(TokenList& tokenList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		LanguageConstruct* bodyConstruct;
		std::string* functionName;
		std::list<std::string>* namedArgumentsList;
	};
}