#pragma once

#include "CppFunctionValue.h"

class WriteLineFunctionValue : public Powder::CppFunctionValue
{
public:
	WriteLineFunctionValue();
	virtual ~WriteLineFunctionValue();

	virtual Powder::Value* Call(Powder::ListValue* argListValue, std::string& errorMsg) override;
};