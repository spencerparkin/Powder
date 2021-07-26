#include "UndefinedValue.h"
#include "BooleanValue.h"
#include "StringValue.h"

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
		switch (mathOp)
		{
			case MathInstruction::EQUAL:
			{
				return new BooleanValue(dynamic_cast<const UndefinedValue*>(value) ? true : false);
			}
			case MathInstruction::NOT_EQUAL:
			{
				return new BooleanValue(dynamic_cast<const UndefinedValue*>(value) ? false : true);
			}
		}

		return new UndefinedValue();
	}

	/*virtual*/ std::string UndefinedValue::ToString() const
	{
		return "UNDEFINED";
	}
}