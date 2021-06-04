#include "Value.h"

namespace Powder
{
	Value::Value()
	{
	}

	/*virtual*/ Value::~Value()
	{
	}

	/*virtual*/ Value* Value::Copy() const
	{
		return nullptr;
	}

	/*virtual*/ Value* Value::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		return nullptr;
	}

	/*virtual*/ std::string Value::ToString() const
	{
		return "?";
	}

	/*virtual*/ bool Value::FromString(const std::string& str)
	{
		return false;
	}

	/*virtual*/ bool Value::AsBoolean() const
	{
		return false;
	}
}