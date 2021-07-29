#pragma once

#include "HashMap.hpp"
#include "GCCollectable.h"
#include "GCSteward.hpp"
#include <vector>

namespace Powder
{
	class Value;

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

		void Absorb(Scope* scope);

		typedef HashMap<Value*> ValueMap;

		ValueMap* GetValueMap() { return &this->valueMap; }

	private:

		GCSteward<Scope> containingScope;
		ValueMap valueMap;
	};
}