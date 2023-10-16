#include "CppFunctionValue.h"
#include "StringValue.h"

namespace Powder
{
	CppFunctionValue::CppFunctionValue()
	{
	}

	/*virtual*/ CppFunctionValue::~CppFunctionValue()
	{
	}

	/*virtual*/ std::string CppFunctionValue::ToString() const
	{
		return "C++ function";
	}

	/*virtual*/ std::string CppFunctionValue::GetTypeString() const
	{
		return "c++function";
	}

	/*virtual*/ std::string CppFunctionValue::GetSetKey() const
	{
		return std::format("c++function:{}", int(this));
	}

	// Note that this function must be updated if the calling convention in the compiler is changed.
	// In this way, I suppose the VM is not entirely indifferent to the compiler that's being used.  Oh well.
	bool CppFunctionValue::CallScriptFunction(AddressValue* addressValue, GC::Reference<ListValue, true>& argListValueRef, GC::Reference<Value, true>& returnValueRef, VirtualMachine* virtualMachine, Error& error)
	{
		return false;
	}
}