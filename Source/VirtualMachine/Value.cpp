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
		// Return a clone of this value.
		return nullptr;
	}

	/*virtual*/ Value* Value::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		// Combine this value with the given value in the given math operation.
		return nullptr;
	}

	/*virtual*/ std::string Value::ToString() const
	{
		// This is the human-readable version of the value.
		return "?";
	}

	/*virtual*/ bool Value::FromString(const std::string& str)
	{
		// If possible, parse the value from the human-readable form.
		return false;
	}

	/*virtual*/ bool Value::AsBoolean() const
	{
		// Return the "truthyness" of this value.
		return false;
	}

	/*virtual*/ double Value::AsNumber() const
	{
		// Return the number equivilant of this value.
		return 0.0f;
	}

	/*virtual*/ std::string Value::GetTypeString() const
	{
		// This is what will be returned by the type() sys-call.
		return "unknown";
	}

	/*virtual*/ std::string Value::GetSetKey() const
	{
		// This is how the value identifies itself in sets.
		return "?";
	}
}