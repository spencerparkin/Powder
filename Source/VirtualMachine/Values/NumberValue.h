#pragma once

#include "Value.h"

namespace Powder
{
	class POWDER_API NumberValue : public Value
	{
	public:
		NumberValue();
		NumberValue(double number);
		virtual ~NumberValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp) const override;

	private:

		double number;
	};
}