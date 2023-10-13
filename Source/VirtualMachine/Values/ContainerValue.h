#pragma once

#include "Value.h"
#include "Reference.h"

namespace Powder
{
	class Error;
	class BooleanValue;
	class CppFunctionValue;

	// This is likely dumb and ill-conceived.
	class POWDER_API ContainerValue : public Value
	{
	public:
		ContainerValue();
		virtual ~ContainerValue();

		virtual bool SetField(Value* fieldValue, Value* dataValue, Error& error);
		virtual Value* GetField(Value* fieldValue, Error& error);
		virtual bool DelField(Value* fieldValue, GC::Reference<Value, true>& valueRef, Error& error);
		virtual BooleanValue* IsMember(const Value* value) const = 0;
		virtual bool AddMember(Value* value, Error& error);
		virtual bool RemoveMember(Value* value, Error& error);
		virtual Value* AnyMember(Error& error);
		virtual CppFunctionValue* MakeIterator(void) = 0;
	};
}