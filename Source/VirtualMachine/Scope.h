#pragma once

#include "HashMap.hpp"
#include "GCReference.hpp"
#include <vector>

namespace Powder
{
	class Value;

	// Scopes provide the main mechanism for facilitating a procedural language, but
	// they can also be used for scope changes within a procedure (local scoping) or
	// for the global scope.  The return address and arguments to a function call
	// are stored in scopes, and therefore, there is no real distinction between
	// local variables of a subroutine and the arguments passed to it.
	class POWDER_API Scope
	{
	public:

		Scope(Scope* containingScope);
		virtual ~Scope();

		Value* LookupValue(const char* identifier);
		void StoreValue(const char* identifier, Value* value);
		void DeleteValue(const char* identifier);
		Scope* GetContainingScope() { return this->containingScope; }

		void LoadValueOntoEvaluationStackTop(const char* identifier);
		void StoreValueFromEvaluationStackTop(const char* identifier);

		void PushValueOntoEvaluationStackTop(Value* value);
		void PopValueFromEvaluationStackTop(Value*& value);

	private:

		Scope* containingScope;

		typedef HashMap<GCReference<Value>> ValueMap;
		ValueMap valueMap;

		std::vector<GCReference<Value>>* evaluationStack;
	};
}