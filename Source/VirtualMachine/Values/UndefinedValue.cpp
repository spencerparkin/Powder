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

	/*virtual*/ Value* UndefinedValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		return new UndefinedValue();
	}

	/*virtual*/ std::string UndefinedValue::ToString() const
	{
		return "undefined";
	}
}