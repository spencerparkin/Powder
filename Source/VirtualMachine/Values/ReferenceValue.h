#pragma once

#include "Value.h"
#include "Reference.h"
#include <string>

namespace Powder
{
	class POWDER_API ReferenceValue : public Value
	{
	public:
		ReferenceValue();
		ReferenceValue(Value* value);
		virtual ~ReferenceValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const override;
		virtual void PopulateIterationArray(std::vector<Object*>& iterationArray) override;

		GC::Reference<Value, false> valueRef;
	};
}