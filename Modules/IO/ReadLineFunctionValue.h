#pragma once

#include "CppFunctionValue.h"

class ReadLineFunctionValue : public Powder::CppFunctionValue
{
public:
	ReadLineFunctionValue();
	virtual ~ReadLineFunctionValue();

	virtual bool Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::CppCallingContext& context, Powder::Error& error) override;
};