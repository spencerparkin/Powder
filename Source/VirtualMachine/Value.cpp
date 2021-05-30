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

	/*virtual*/ Value* Value::CombineWith(const Value* value, MathInstruction::MathOp mathOp) const
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
}