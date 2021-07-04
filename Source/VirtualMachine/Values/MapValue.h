#pragma once

#include "ContainerValue.h"
#include "HashMap.hpp"

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
		virtual void SetField(Value* fieldValue, Value* dataValue) override;
		virtual Value* GetField(Value* fieldValue) override;
		virtual Value* DelField(Value* fieldValue) override;
		virtual BooleanValue* IsMember(const Value* value) const override;

		ListValue* GenerateKeyListValue();

		void SetField(const char* key, Value* dataValue);
		Value* GetField(const char* key);

	private:
		HashMap<Value*> valueMap;
	};
}