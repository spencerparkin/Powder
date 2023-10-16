#pragma once

#include "CppFunctionValue.h"

class NewComplexValueFunction : public Powder::CppFunctionValue
{
public:
	NewComplexValueFunction();
	virtual ~NewComplexValueFunction();

	virtual bool Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::VirtualMachine* virtualMachine, Powder::Error& error) override;
};