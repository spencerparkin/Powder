#include "NumberValue.h"
#include "UndefinedValue.h"

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

	/*virtual*/ Value* NumberValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		const NumberValue* numberValue = dynamic_cast<const NumberValue*>(value);
		if (numberValue)
		{
			switch (mathOp)
			{
				case MathInstruction::MathOp::ADD:
				{
					return new NumberValue(this->number + numberValue->number);
				}
				case MathInstruction::MathOp::SUBTRACT:
				{
					return new NumberValue(this->number - numberValue->number);
				}
				case MathInstruction::MathOp::MULTIPLY:
				{
					return new NumberValue(this->number * numberValue->number);
				}
				case MathInstruction::MathOp::DIVIDE:
				{
					return new NumberValue(this->number / numberValue->number);
				}
			}
		}

		return new UndefinedValue();
	}
}