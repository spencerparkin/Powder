#pragma once

#include "Value.h"

namespace Powder
{
	class Error;
	class ListValue;

	class POWDER_API CppFunctionValue : public Value
	{
	public:
		CppFunctionValue();
		virtual ~CppFunctionValue();

		virtual bool Call(ListValue* argListValue, GC::Reference<Value, true>& returnValueRef, Error& error) = 0;
		virtual std::string ToString() const override;
		virtual std::string GetTypeString() const override;
		virtual std::string GetSetKey() const override;
	};
}