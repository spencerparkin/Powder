#pragma once

#include "HashMap.hpp"
#include "GCReference.hpp"
#include <vector>

namespace Powder
{
	class Value;

	// The name here may be an abuse of the term.  A perhaps more accurate
	// term here is call stack frame.
	class POWDER_API Scope
	{
	public:

		Scope(Scope* containingScope);
		virtual ~Scope();

		Value* LookupValue(const char* identifier, bool canPropagateSearch);
		void StoreValue(const char* identifier, Value* value);
		void DeleteValue(const char* identifier);

		Scope* GetContainingScope() { return this->containingScope; }

		void LoadValueOntoEvaluationStackTop(const char* identifier);
		void StoreValueFromEvaluationStackTop(const char* identifier);

		void PushValueOntoEvaluationStackTop(Value* value);
		Value* PopValueFromEvaluationStackTop();

	private:

		Scope* containingScope;

		typedef HashMap<GCReference<Value>> ValueMap;
		ValueMap valueMap;

		std::vector<GCReference<Value>>* evaluationStack;
	};
}