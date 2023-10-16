#pragma once

#include "Value.h"
#include "Reference.h"

namespace Powder
{
	class Error;
	class ListValue;
	class VirtualMachine;
	class AddressValue;

	class POWDER_API CppFunctionValue : public Value
	{
	public:
		CppFunctionValue();
		virtual ~CppFunctionValue();

		virtual bool Call(ListValue* argListValue, GC::Reference<Value, true>& returnValueRef, VirtualMachine* virtualMachine, Error& error) = 0;
		virtual std::string ToString() const override;
		virtual std::string GetTypeString() const override;
		virtual std::string GetSetKey() const override;

		bool CallScriptFunction(AddressValue* addressValue, GC::Reference<ListValue, true>& argListValueRef, GC::Reference<Value, true>& returnValueRef, VirtualMachine* virtualMachine, Error& error);
	};
}