#pragma once

#include "Value.h"
#include "Reference.h"

namespace Powder
{
	class Error;
	class ListValue;
	class VirtualMachine;
	class Executor;
	class AddressValue;

	struct POWDER_API CppCallingContext
	{
		Executor* executor;
		VirtualMachine* virtualMachine;
	};

	class POWDER_API CppFunctionValue : public Value
	{
	public:
		CppFunctionValue();
		virtual ~CppFunctionValue();

		virtual bool Call(ListValue* argListValue, GC::CriticalReference<Value>& returnValueRef, CppCallingContext& context, Error& error) = 0;
		virtual std::string ToString() const override;
		virtual std::string GetTypeString() const override;
		virtual std::string GetSetKey() const override;

		// If this call returns false, then the caller must also return false as the state of the VM is left undefined.
		bool CallScriptFunction(AddressValue* addressValue, GC::Reference<ListValue, true>& argListValueRef, GC::CriticalReference<Value>& returnValueRef, CppCallingContext& context, Error& error);
	};
}