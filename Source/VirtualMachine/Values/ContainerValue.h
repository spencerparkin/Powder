#pragma once

#include "Value.h"
#include "Reference.h"

namespace Powder
{
	class Error;
	class BooleanValue;
	class CppFunctionValue;

	class POWDER_API ContainerValue : public Value
	{
	public:
		ContainerValue();
		virtual ~ContainerValue();

		virtual bool SetField(Value* fieldValue, Value* dataValue, Error& error) = 0;
		virtual Value* GetField(Value* fieldValue, Error& error) = 0;
		virtual bool DelField(Value* fieldValue, GC::Reference<Value, true>& valueRef, Error& error) = 0;
		virtual BooleanValue* IsMember(const Value* value) const = 0;
		virtual CppFunctionValue* MakeIterator(void) = 0;
	};
}