#include "Scope.h"
#include "Value.h"

namespace Powder
{
	Scope::Scope()
	{
	}

	/*virtual*/ Scope::~Scope()
	{
		this->valueMap.Clear();
	}

	Scope* Scope::GetContainingScope()
	{
		return this->containingScopeRef.Get();
	}

	void Scope::SetContainingScope(Scope* containingScope)
	{
		this->containingScopeRef.Set(containingScope);
	}

	Value* Scope::LookupValue(const char* identifier, bool canPropagateSearch)
	{
		Scope* scope = this;
		while (scope)
		{
			GC::Reference<Value, false>* valueRef = scope->valueMap.LookupPtr(identifier);
			if (valueRef)
				return valueRef->Get();

			if (canPropagateSearch)
				scope = scope->GetContainingScope();
			else
				break;
		}

		return nullptr;
	}

	void Scope::StoreValue(const char* identifier, Value* value)
	{
		GC::Reference<Value, false>* valueRef = this->valueMap.LookupPtr(identifier);
		if (valueRef)
		{
			valueRef->Set(value);
			return;
		}

		this->valueMap.Insert(identifier, value);
	}

	void Scope::DeleteValue(const char* identifier)
	{
		this->valueMap.Remove(identifier);
	}

	void Scope::Absorb(Scope* scope)
	{
		scope->valueMap.ForAllEntries([this](const char* key, GC::Reference<Value, false>& valueRef) -> bool {
			this->StoreValue(key, valueRef.Get());
			return true;
		});
	}

	/*virtual*/ void Scope::PopulateIterationArray(std::vector<GC::Object*>& iterationArray)
	{
		iterationArray.push_back(&this->containingScopeRef);
		for (GC::Reference<Value, false>& valueRef : this->valueMap)
			iterationArray.push_back(&valueRef);
	}
}