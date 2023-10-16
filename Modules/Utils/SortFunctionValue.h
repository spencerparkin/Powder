#pragma once

#include "CppFunctionValue.h"

class SortFunctionValue : public Powder::CppFunctionValue
{
public:
	SortFunctionValue();
	virtual ~SortFunctionValue();

	virtual bool Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::CppCallingContext& context, Powder::Error& error) override;
};