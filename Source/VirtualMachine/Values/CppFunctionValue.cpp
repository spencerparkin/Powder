#include "CppFunctionValue.h"
#include "StringValue.h"

namespace Powder
{
	CppFunctionValue::CppFunctionValue()
	{
	}

	/*virtual*/ CppFunctionValue::~CppFunctionValue()
	{
	}

	/*virtual*/ std::string CppFunctionValue::ToString() const
	{
		return "C++ function";
	}

	/*virtual*/ std::string CppFunctionValue::GetTypeString() const
	{
		return "c++function";
	}
}