#include "Scope.h"
#include "Value.h"

namespace Powder
{
	Scope::Scope(Scope* containingScope)
	{
		this->containingScope = containingScope;
	}

	/*virtual*/ Scope::~Scope()
	{
		this->valueMap.Clear();
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
}