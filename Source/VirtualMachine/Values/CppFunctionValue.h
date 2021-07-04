#pragma once

#include "Value.h"

namespace Powder
{
	class ListValue;

	class POWDER_API CppFunctionValue : public Value
	{
	public:
		CppFunctionValue();
		virtual ~CppFunctionValue();

		virtual Value* Call(ListValue* argListValue, std::string& errorMsg) = 0;
	};
}