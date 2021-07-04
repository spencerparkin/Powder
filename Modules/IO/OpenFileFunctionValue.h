#pragma once

#include "CppFunctionValue.h"

class OpenFileFunctionValue : public Powder::CppFunctionValue
{
public:
	OpenFileFunctionValue();
	virtual ~OpenFileFunctionValue();

	virtual Powder::Value* Call(Powder::ListValue* argListValue, std::string& errorMsg) override;
};