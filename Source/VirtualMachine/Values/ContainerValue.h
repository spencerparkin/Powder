#pragma once

#include "Value.h"
#include "Reference.h"

namespace Powder
{
	class BooleanValue;
	class CppFunctionValue;

	class POWDER_API ContainerValue : public Value
	{
	public:
		ContainerValue();
		virtual ~ContainerValue();

		virtual void SetField(Value* fieldValue, Value* dataValue) = 0;
		virtual Value* GetField(Value* fieldValue) = 0;
		virtual bool DelField(Value* fieldValue, GC::Reference<Value, true>& valueRef) = 0;
		virtual BooleanValue* IsMember(const Value* value) const = 0;
		virtual CppFunctionValue* MakeIterator(void) = 0;
	};
}