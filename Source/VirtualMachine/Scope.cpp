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

	Value* Scope::LookupValue(const char* identifier)
	{
		Value* value = this->valueMap.Lookup(identifier);
		
		if (!value && this->containingScope)
			value = this->containingScope->LookupValue(identifier);

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

	bool Scope::LoadValueOntoEvaluationStackTop(const char* identifier)
	{
		Value* value = this->LookupValue(identifier);
		if (!value)
			return false;

		return this->PushValueOntoEvaluationStackTop(value);
	}

	bool Scope::StoreValueFromEvaluationStackTop(const char* identifier)
	{
		Value* value = nullptr;
		if (!this->PopValueFromEvaluationStackTop(value))
			return false;

		this->StoreValue(identifier, value);
		return true;
	}

	bool Scope::PushValueOntoEvaluationStackTop(Value* value)
	{
		this->evaluationStack->push_back(value);
		return true;
	}

	bool Scope::PopValueFromEvaluationStackTop(Value*& value)
	{
		value = nullptr;

		if (this->evaluationStack->size() == 0)
			return false;

		value = (*this->evaluationStack)[this->evaluationStack->size() - 1];
		this->evaluationStack->pop_back();
		return true;
	}
}