#pragma once

#include "CppFunctionValue.h"

class ReadLineFunctionValue : public Powder::CppFunctionValue
{
public:
	ReadLineFunctionValue();
	virtual ~ReadLineFunctionValue();

	virtual Powder::Value* Call(Powder::ListValue* argListValue, std::string& errorMsg) override;
};