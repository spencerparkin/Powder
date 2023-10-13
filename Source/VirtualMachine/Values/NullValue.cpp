#include "NullValue.h"
#include "BooleanValue.h"
#include "StringValue.h"

namespace Powder
{
	NullValue::NullValue()
	{
	}

	/*virtual*/ NullValue::~NullValue()
	{
	}

	/*virtual*/ Value* NullValue::Copy() const
	{
		return new NullValue();
	}

	/*virtual*/ std::string NullValue::GetTypeString() const
	{
		return "null";
	}

	/*virtual*/ Value* NullValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		switch (mathOp)
		{
			case MathInstruction::EQUAL:
			{
				return new BooleanValue(dynamic_cast<const NullValue*>(value) ? true : false);
			}
			case MathInstruction::NOT_EQUAL:
			{
				return new BooleanValue(dynamic_cast<const NullValue*>(value) ? false : true);
			}
		}

		return new NullValue();
	}

	/*virtual*/ std::string NullValue::ToString() const
	{
		return "NULL";
	}
}