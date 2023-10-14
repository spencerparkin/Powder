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
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const override;
		virtual bool FromString(const std::string& str) override;
		virtual bool AsBoolean() const override;
		virtual double AsNumber() const override;
		virtual std::string GetTypeString() const override;
		virtual std::string GetSetKey() const override;

	private:
		double number;
	};
}