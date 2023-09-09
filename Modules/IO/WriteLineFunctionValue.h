#pragma once

#include "CppFunctionValue.h"

class WriteLineFunctionValue : public Powder::CppFunctionValue
{
public:
	WriteLineFunctionValue();
	virtual ~WriteLineFunctionValue();

	virtual bool Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::Error& error) override;
};