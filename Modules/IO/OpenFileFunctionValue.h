#pragma once

#include "CppFunctionValue.h"

class OpenFileFunctionValue : public Powder::CppFunctionValue
{
public:
	OpenFileFunctionValue();
	virtual ~OpenFileFunctionValue();

	virtual bool Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::CppCallingContext& context, Powder::Error& error) override;
};