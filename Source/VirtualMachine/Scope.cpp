#include "Scope.h"
#include "Value.h"
#include "Exceptions.hpp"

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

	Value* Scope::LookupValue(const char* identifier, bool canPropagateSearch)
	{
		Value* value = nullptr;
		Scope* scope = this;
		while (scope)
		{
			value = scope->valueMap.Lookup(identifier);
			if (value)
				break;

			if (canPropagateSearch)
				scope = scope->GetContainingScope();
			else
				break;
		}

		return value;
	}

	void Scope::StoreValue(const char* identifier, Value* value)
	{
		this->valueMap.Insert(identifier, value);
	}

	void Scope::DeleteValue(const char* identifier)
	{
		this->valueMap.Remove(identifier);
	}

	void Scope::LoadAndPushValueOntoEvaluationStackTop(const char* identifier)
	{
		Value* value = this->LookupValue(identifier, false);
		if (!value)
			throw new RunTimeException(FormatString("Failed to lookup identifier: %s", identifier));

		return this->PushValueOntoEvaluationStackTop(value);
	}

	void Scope::StoreAndPopValueFromEvaluationStackTop(const char* identifier)
	{
		Value* value = this->PopValueFromEvaluationStackTop();
		this->StoreValue(identifier, value);
	}

	void Scope::PushValueOntoEvaluationStackTop(Value* value)
	{
		this->evaluationStack->push_back(value);
	}

	Value* Scope::PopValueFromEvaluationStackTop()
	{
		Value* value = nullptr;
		if (this->evaluationStack->size() == 0)
			throw new RunTimeException("Evaluation stack underflow!");
		value = (*this->evaluationStack)[this->evaluationStack->size() - 1];
		this->evaluationStack->pop_back();
		return value;
	}
}