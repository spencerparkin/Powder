#pragma once

#include "CppFunctionValue.h"

class CloseFileFunctionValue : public Powder::CppFunctionValue
{
public:
	CloseFileFunctionValue();
	virtual ~CloseFileFunctionValue();

	virtual Powder::Value* Call(Powder::ListValue* argListValue, std::string& errorMsg) override;
};