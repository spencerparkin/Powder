#pragma once

#include "Value.h"

namespace Powder
{
	class POWDER_API UndefinedValue : public Value
	{
	public:
		UndefinedValue();
		virtual ~UndefinedValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp) const override;
	};
}