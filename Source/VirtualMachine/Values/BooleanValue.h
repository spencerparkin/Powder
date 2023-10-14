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
		virtual std::string ToString() const override;
		virtual bool AsBoolean() const override;
		virtual std::string GetTypeString() const override;
		virtual std::string GetSetKey() const override;

		void SetBool(bool boolValue) { this->boolValue = boolValue; }
		bool GetBool() const { return this->boolValue; }

	private:
		
		bool boolValue;
	};
}