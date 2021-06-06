#pragma once

#include "Value.h"

namespace Powder
{
	class POWDER_API BooleanValue : public Value
	{
	public:
		BooleanValue();
		BooleanValue(bool boolValue);
		virtual ~BooleanValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual bool AsBoolean() const override;

	private:
		
		bool boolValue;
	};
}