#include "CppFunctionValue.h"
#include "StringValue.h"
#include "ListValue.h"
#include "AddressValue.h"
#include "VirtualMachine.h"
#include "Executor.h"

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
	bool CppFunctionValue::CallScriptFunction(AddressValue* addressValue, GC::Reference<ListValue, true>& argListValueRef, GC::Reference<Value, true>& returnValueRef, CppCallingContext& context, Error& error)
	{
		int stackSizeBeforeCall = context.executor->StackSize();

		// The script function will be popping this off the eval stack to populate its arguments.
		if (!context.executor->PushValueOntoEvaluationStackTop(argListValueRef.Get(), error))
			return false;

		// We need to push scope for the call so that the scope handed to the function is in the scope chain.
		if (!context.executor->PushScope())
			return false;

		Scope* scope = context.executor->GetCurrentScope();
		if (!scope)
			return false;

		GC::Reference<AddressValue, true> returnAddressValueRef(new AddressValue());
		returnAddressValueRef.Get()->executableRef.Set(const_cast<Executable*>(context.executor->GetExecutable()));
		returnAddressValueRef.Get()->programBufferLocation = context.executor->GetProgramBufferLocation();
		returnAddressValueRef.Get()->cppReturn = true;
		scope->StoreValue("__return_address__", returnAddressValueRef.Get());

		context.executor->SetProgramBufferLocation(addressValue->programBufferLocation);
		context.executor->SetExecutable(addressValue->executableRef.Get());

		GC::Reference<Executor, true> executorRef(context.executor);
		if (!context.virtualMachine->ExecuteByteCode(executorRef, error))
			return false;

		if (!context.executor->PopScope())
			return false;

		if (!context.executor->PopValueFromEvaluationStackTop(returnValueRef, error))
			return false;

		int stackSizeAfterCall = context.executor->StackSize();
		if (stackSizeAfterCall != stackSizeBeforeCall)
		{
			error.Add("Call from C++ to script didn't leave stack-size the same.");
			return false;
		}

		return true;
	}
}