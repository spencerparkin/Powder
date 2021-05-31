#include "BooleanValue.h"

namespace Powder
{
	BooleanValue::BooleanValue()
	{
		this->boolValue = false;
	}

	BooleanValue::BooleanValue(bool boolValue)
	{
		this->boolValue = boolValue;
	}

	/*virtual*/ BooleanValue::~BooleanValue()
	{
	}

	/*virtual*/ Value* BooleanValue::Copy() const
	{
		return new BooleanValue(this->boolValue);
	}

	/*virtual*/ bool BooleanValue::AsBoolean() const
	{
		return this->boolValue;
	}
}