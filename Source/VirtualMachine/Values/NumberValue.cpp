#include "NumberValue.h"
#include "NullValue.h"
#include "StringValue.h"
#include "BooleanValue.h"

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
		switch (mathOp)
		{
			case MathInstruction::MathOp::FACTORIAL:
			{
				// TODO: There is actually a factorial for non-integers, and why not support it?  It might not be trivial to calculate, though.
				double rounded = double(uint32_t(this->number));
				double factorial = 1.0;
				while (rounded-- > 0.0)
					factorial *= rounded;
				return new NumberValue(factorial);
			}
			case MathInstruction::MathOp::NEGATE:
			{
				return new NumberValue(-this->number);
			}
		}

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
				case MathInstruction::MathOp::MODULUS:
				{
					return new NumberValue(::fmod(this->number, numberValue->number));
				}
				case MathInstruction::MathOp::EQUAL:
				{
					return new BooleanValue(this->number == numberValue->number);
				}
				case MathInstruction::MathOp::NOT_EQUAL:
				{
					return new BooleanValue(this->number != numberValue->number);
				}
				case MathInstruction::MathOp::LESS_THAN:
				{
					return new BooleanValue(this->number < numberValue->number);
				}
				case MathInstruction::MathOp::LESS_THAN_OR_EQUAL:
				{
					return new BooleanValue(this->number <= numberValue->number);
				}
				case MathInstruction::MathOp::GREATER_THAN:
				{
					return new BooleanValue(this->number > numberValue->number);
				}
				case MathInstruction::MathOp::GREATER_THAN_OR_EQUAL:
				{
					return new BooleanValue(this->number >= numberValue->number);
				}
			}

			return new NullValue();
		}
		
		if (mathOp == MathInstruction::MathOp::ADD)
		{
			const StringValue* stringValue = dynamic_cast<const StringValue*>(value);
			if (stringValue)
				return new StringValue(this->ToString() + stringValue->ToString());
		}

		return new NullValue();
	}

	/*virtual*/ std::string NumberValue::ToString() const
	{
		return std::format("{}", this->number);
	}

	/*virtual*/ bool NumberValue::FromString(const std::string& str)
	{
		char* endPtr = nullptr;
		this->number = ::strtod(str.c_str(), &endPtr);
		if (endPtr == str.c_str())
			return false;
		return true;
	}

	/*virtual*/ bool NumberValue::AsBoolean() const
	{
		return this->number != 0.0;
	}

	/*virtual*/ double NumberValue::AsNumber() const
	{
		return this->number;
	}
}