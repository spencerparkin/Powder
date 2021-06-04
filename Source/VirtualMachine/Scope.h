#pragma once

#include "HashMap.hpp"
#include "GCReference.hpp"
#include "GCCollectable.h"
#include "GCSteward.hpp"
#include <vector>

namespace Powder
{
	class Value;

	// The name here may be an abuse of the term.  A perhaps more accurate
	// term here is call stack frame.
	class POWDER_API Scope : public GCCollectable
	{
	public:

		Scope();
		virtual ~Scope();

		Value* LookupValue(const char* identifier, bool canPropagateSearch);
		void StoreValue(const char* identifier, Value* value);
		void DeleteValue(const char* identifier);

		Scope* GetContainingScope();
		void SetContainingScope(Scope* containingScope);

		void LoadAndPushValueOntoEvaluationStackTop(const char* identifier);
		void StoreAndPopValueFromEvaluationStackTop(const char* identifier);

		void PushValueOntoEvaluationStackTop(Value* value);
		Value* PopValueFromEvaluationStackTop();

	private:

		GCSteward<Scope> containingScope;

		typedef HashMap<GCReference<Value>> ValueMap;
		ValueMap valueMap;

		std::vector<GCReference<Value>>* evaluationStack;
	};
}