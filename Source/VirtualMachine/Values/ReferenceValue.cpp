#include "ReferenceValue.h"
#include "BooleanValue.h"
#include "NullValue.h"

using namespace Powder;

ReferenceValue::ReferenceValue()
{
}

ReferenceValue::ReferenceValue(Value* value)
{
	this->valueRef.Set(value);
}

/*virtual*/ ReferenceValue::~ReferenceValue()
{
}

/*virtual*/ Value* ReferenceValue::Copy() const
{
	return new ReferenceValue(const_cast<Value*>(this->valueRef.Get()));
}

/*virtual*/ Value* ReferenceValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
{
	const ReferenceValue* refValue = dynamic_cast<const ReferenceValue*>(value);
	if (refValue)
	{
		switch (mathOp)
		{
			case MathInstruction::MathOp::EQUAL:
			{
				return new BooleanValue(this->valueRef.Get() == refValue->valueRef.Get());
			}
			case MathInstruction::MathOp::NOT_EQUAL:
			{
				return new BooleanValue(this->valueRef.Get() != refValue->valueRef.Get());
			}
		}
	}

	return new NullValue();
}

/*virtual*/ std::string ReferenceValue::ToString() const
{
	return std::format("{}", uint64_t(this->valueRef.Get()));
}

/*virtual*/ void ReferenceValue::PopulateIterationArray(std::vector<Object*>& iterationArray)
{
	iterationArray.push_back(&this->valueRef);
}