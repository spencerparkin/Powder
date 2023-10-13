#include "BooleanValue.h"
#include "NullValue.h"

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

	/*virtual*/ std::string BooleanValue::ToString() const
	{
		return this->boolValue ? "true" : "false";
	}

	/*virtual*/ std::string BooleanValue::GetTypeString() const
	{
		return "boolean";
	}

	/*virtual*/ Value* BooleanValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		if (!value)
		{
			switch (mathOp)
			{
				case MathInstruction::MathOp::NOT:
				{
					return new BooleanValue(!this->boolValue);
				}
			}
		}

		const BooleanValue* booleanValue = dynamic_cast<const BooleanValue*>(value);
		if (booleanValue)
		{
			switch (mathOp)
			{
				case MathInstruction::MathOp::AND:
				{
					return new BooleanValue(this->boolValue && booleanValue->boolValue);
				}
				case MathInstruction::MathOp::OR:
				{
					return new BooleanValue(this->boolValue || booleanValue->boolValue);
				}
				case MathInstruction::MathOp::XOR:
				{
					return new BooleanValue((this->boolValue || booleanValue->boolValue) && this->boolValue != booleanValue->boolValue);
				}
			}
		}

		return new NullValue();
	}

	/*virtual*/ bool BooleanValue::AsBoolean() const
	{
		return this->boolValue;
	}
}