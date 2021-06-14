#include "AddressValue.h"
#include "UndefinedValue.h"

namespace Powder
{
	AddressValue::AddressValue()
	{
		this->programBufferLocation = 0;
	}

	AddressValue::AddressValue(uint64_t programBufferLocation)
	{
		this->programBufferLocation = programBufferLocation;
	}

	/*virtual*/ AddressValue::~AddressValue()
	{
	}

	/*virtual*/ Value* AddressValue::Copy() const
	{
		return new AddressValue(this->programBufferLocation);
	}

	/*virtual*/ Value* AddressValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		return new UndefinedValue();
	}
}