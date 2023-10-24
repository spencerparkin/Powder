#pragma once

#include "CppFunctionValue.h"

namespace Powder
{
	class POWDER_API RangeIteratorValue : public CppFunctionValue
	{
	public:
		RangeIteratorValue(double min, double max, double step);
		virtual ~RangeIteratorValue();

		virtual bool Call(ListValue* argListValue, GC::Reference<Value, true>& returnValueRef, CppCallingContext& context, Error& error) override;
		virtual std::string ToString() const override;
		virtual std::string GetTypeString() const override;
		virtual std::string GetSetKey() const override;

	private:
		double start, stop, step;
		double currentNumber;
		bool nextExists;
	};
}