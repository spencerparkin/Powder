#include "RangeIteratorValue.h"
#include "ListValue.h"
#include "NullValue.h"
#include "NumberValue.h"

using namespace Powder;

RangeIteratorValue::RangeIteratorValue(double start, double stop, double step)
{
	this->start = start;
	this->stop = stop;
	this->step = step;
	this->currentNumber = 0.0;
	this->nextExists = false;
}

/*virtual*/ RangeIteratorValue::~RangeIteratorValue()
{
}

/*virtual*/ bool RangeIteratorValue::Call(ListValue* argListValue, GC::CriticalReference<Value>& returnValueRef, CppCallingContext& context, Error& error)
{
	if (argListValue->Length() != 1)
	{
		error.Add("Range iterator function expected exactly one argument.");
		return false;
	}

	const Value* actionValue = (*dynamic_cast<ListValue*>(argListValue))[0];
	std::string actionStr = actionValue->ToString();

	if (actionStr == "reset")
	{
		this->currentNumber = this->start;
		this->nextExists = true;

		if (::signbit(this->stop - this->start) != ::signbit(this->step))
		{
			error.Add(std::format("Iteration from {} to {} by step {} will never terminate.", this->start, this->stop, this->step));
			return false;
		}
	}
	else if (actionStr == "next")
	{
		if (!this->nextExists)
			returnValueRef.Set(new NullValue());
		else
		{
			returnValueRef.Set(new NumberValue(this->currentNumber));
			this->nextExists = !(::fabs(this->currentNumber - this->stop) < this->step);
			this->currentNumber += this->step;
		}
	}

	return true;
}

/*virtual*/ std::string RangeIteratorValue::ToString() const
{
	return std::format("Range iterator: [start={}, stop={}, step={}]", this->start, this->stop, this->step);
}

/*virtual*/ std::string RangeIteratorValue::GetTypeString() const
{
	return "range";
}

/*virtual*/ std::string RangeIteratorValue::GetSetKey() const
{
	return std::format("range:{}", uintptr_t(this));
}