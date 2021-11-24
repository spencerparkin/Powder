#include "Scope.h"
#include "Value.h"
#include "Exceptions.hpp"

namespace Powder
{
	Scope::Scope() : containingScope(this)
	{
	}

	/*virtual*/ Scope::~Scope()
	{
		this->valueMap.Clear();
	}

	Scope* Scope::GetContainingScope()
	{
		return this->containingScope.Get();
	}

	void Scope::SetContainingScope(Scope* containingScope)
	{
		this->containingScope.Set(containingScope);
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
		Value* existingValue = this->valueMap.Lookup(identifier);
		if (existingValue)
		{
			if (value == existingValue)
				return;

			this->DisconnectFrom(existingValue);
		}

		this->valueMap.Insert(identifier, value);

		if (value)
			this->ConnectTo(value);
	}

	void Scope::DeleteValue(const char* identifier)
	{
		Value* existingValue = this->valueMap.Lookup(identifier);
		if (existingValue)
			this->DisconnectFrom(existingValue);

		this->valueMap.Remove(identifier);
	}

	void Scope::Absorb(Scope* scope)
	{
		scope->valueMap.ForAllEntries([this](const char* key, Value* value) -> bool {
			this->StoreValue(key, value);
			return true;
		});
	}
}