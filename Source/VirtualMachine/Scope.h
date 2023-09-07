#pragma once

#include "HashMap.hpp"
#include "Collectable.h"
#include "Reference.h"
#include <vector>

namespace Powder
{
	class Value;

	class POWDER_API Scope : public GC::Collectable
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

		typedef HashMap<GC::Reference<Value, false>> ValueMap;

		ValueMap* GetValueMap() { return &this->valueMap; }

		virtual void PopulateIterationArray(std::vector<Object*>& iterationArray) override;

	private:

		GC::Reference<Scope, false> containingScopeRef;

		ValueMap valueMap;
	};
}