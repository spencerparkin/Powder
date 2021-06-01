#pragma once

#include "LanguageConstruct.h"

namespace Powder
{
	class POWDER_API FunctionCallConstruct : public LanguageConstruct
	{
	public:
		FunctionCallConstruct();
		virtual ~FunctionCallConstruct();

		virtual LanguageConstruct* New() override;
		virtual bool Parse(TokenList& tokenList) override;
		virtual void GenerateInstructionSequence(LinkedList<Instruction*>& instructionList) override;

	protected:
		
		LinkedList<LanguageConstruct*> argumentConstructList;

		std::string* functionName;
	};
}