#pragma once

#include "Value.h"

namespace Powder
{
	class POWDER_API VariableValue : public Value
	{
	public:
		VariableValue();
		VariableValue(const std::string& identifier);
		virtual ~VariableValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;

	private:

		std::string* identifier;
	};
}