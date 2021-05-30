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

	void Scope::LoadValueOntoEvaluationStackTop(const char* identifier)
	{
		Value* value = this->LookupValue(identifier, false);
		if (!value)
		{
			// TODO: Throw an exception.
		}

		return this->PushValueOntoEvaluationStackTop(value);
	}

	void Scope::StoreValueFromEvaluationStackTop(const char* identifier)
	{
		Value* value = nullptr;
		this->PopValueFromEvaluationStackTop(value);
		this->StoreValue(identifier, value);
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