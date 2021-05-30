#include "NumberValue.h"

namespace Powder
{
	NumberValue::NumberValue()
	{
		this->number = 0.0;
	}

	NumberValue::NumberValue(double number)
	{
		this->number = number;
	}

	/*virtual*/ NumberValue::~NumberValue()
	{
	}

	/*virtual*/ Value* NumberValue::Copy() const
	{
		return new NumberValue(this->number);
	}
}