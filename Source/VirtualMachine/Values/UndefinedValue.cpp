#include "UndefinedValue.h"

namespace Powder
{
	UndefinedValue::UndefinedValue()
	{
	}

	/*virtual*/ UndefinedValue::~UndefinedValue()
	{
	}

	/*virtual*/ Value* UndefinedValue::Copy() const
	{
		return new UndefinedValue();
	}
}