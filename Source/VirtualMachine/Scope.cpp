#include "Scope.h"
#include "Value.h"

namespace Powder
{
	Scope::Scope(Scope* containingScope)
	{
		this->containingScope = containingScope;
		this->evaluationStack = new std::vector<GCReference<Value>>();
	}

	/*virtual*/ Scope::~Scope()
	{
		this->valueMap.Clear();
		delete this->evaluationStack;
	}

	Scope* Scope::FindScopeAtLevel(uint32_t scopeLevel)
	{
		Scope* scope = this;
		while (scopeLevel-- > 0)
		{
			scope = scope->containingScope;
			if (!scope)
			{
				// TODO: Throw an exception.
			}
		}

		return scope;
	}

	Value* Scope::LookupValue(const char* identifier, uint32_t scopeLevel)
	{
		return this->FindScopeAtLevel(scopeLevel)->valueMap.Lookup(identifier);
	}

	void Scope::StoreValue(const char* identifier, Value* value, uint32_t scopeLevel)
	{
		this->FindScopeAtLevel(scopeLevel)->valueMap.Insert(identifier, value);
	}

	void Scope::DeleteValue(const char* identifier, uint32_t scopeLevel)
	{
		this->FindScopeAtLevel(scopeLevel)->valueMap.Remove(identifier);
	}

	void Scope::LoadValueOntoEvaluationStackTop(const char* identifier, uint32_t scopeLevel)
	{
		Value* value = this->LookupValue(identifier, scopeLevel);
		if (!value)
		{
			// TODO: Throw an exception.
		}

		return this->PushValueOntoEvaluationStackTop(value);
	}

	void Scope::StoreValueFromEvaluationStackTop(const char* identifier, uint32_t scopeLevel)
	{
		Value* value = nullptr;
		this->PopValueFromEvaluationStackTop(value);
		this->StoreValue(identifier, value, scopeLevel);
	}

	void Scope::PushValueOntoEvaluationStackTop(Value* value)
	{
		this->evaluationStack->push_back(value);
	}

	void Scope::PopValueFromEvaluationStackTop(Value*& value)
	{
		value = nullptr;

		if (this->evaluationStack->size() == 0)
		{
			// TODO: Throw an exception.
		}

		value = (*this->evaluationStack)[this->evaluationStack->size() - 1];
		this->evaluationStack->pop_back();
	}
}