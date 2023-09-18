#pragma once

#include "Value.h"

namespace Powder
{
	class POWDER_API NullValue : public Value
	{
	public:
		NullValue();
		virtual ~NullValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const;
	};
}