#pragma once

#include "ContainerValue.h"
#include "CppFunctionValue.h"
#include "HashMap.hpp"
#include "GCSteward.hpp"

namespace Powder
{
	class ListValue;

	class POWDER_API MapValue : public ContainerValue
	{
	public:
		MapValue();
		virtual ~MapValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const override;
		virtual void SetField(Value* fieldValue, Value* dataValue, bool decRefAfterSet) override;
		virtual Value* GetField(Value* fieldValue) override;
		virtual Value* DelField(Value* fieldValue, bool incRefBeforeDel) override;
		virtual BooleanValue* IsMember(const Value* value) const override;
		virtual CppFunctionValue* MakeIterator(void) override;

		ListValue* GenerateKeyListValue();

		void SetField(const char* key, Value* dataValue, bool decRefAfterSet);
		Value* GetField(const char* key);
		Value* DelField(const char* key, bool incRefBeforeDel);

		HashMap<Value*>& GetValueMap() { return this->valueMap; }
		const HashMap<Value*>& GetValueMap() const { return this->valueMap; }

	private:
		HashMap<Value*> valueMap;
	};

	class POWDER_API MapValueIterator : public CppFunctionValue
	{
	public:
		MapValueIterator(MapValue* mapValue);
		virtual ~MapValueIterator();

		virtual Value* Call(ListValue* argListValue, std::string& errorMsg) override;

		GCSteward<MapValue> mapValue;
		HashMap<Value*>::iterator mapIter;
	};
}