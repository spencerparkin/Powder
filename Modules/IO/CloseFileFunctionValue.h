#pragma once

#include "CppFunctionValue.h"

class CloseFileFunctionValue : public Powder::CppFunctionValue
{
public:
	CloseFileFunctionValue();
	virtual ~CloseFileFunctionValue();

	virtual bool Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::VirtualMachine* virtualMachine, Powder::Error& error) override;
};