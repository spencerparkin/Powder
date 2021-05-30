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

		Value* LookupValue(const char* identifier, uint32_t scopeLevel);
		void StoreValue(const char* identifier, Value* value, uint32_t scopeLevel);
		void DeleteValue(const char* identifier, uint32_t scopeLevel);

		Scope* GetContainingScope() { return this->containingScope; }

		void LoadValueOntoEvaluationStackTop(const char* identifier, uint32_t scopeLevel);
		void StoreValueFromEvaluationStackTop(const char* identifier, uint32_t scopeLevel);

		void PushValueOntoEvaluationStackTop(Value* value);
		void PopValueFromEvaluationStackTop(Value*& value);

	private:

		Scope* FindScopeAtLevel(uint32_t scopeLevel);

		Scope* containingScope;

		typedef HashMap<GCReference<Value>> ValueMap;
		ValueMap valueMap;

		std::vector<GCReference<Value>>* evaluationStack;
	};
}