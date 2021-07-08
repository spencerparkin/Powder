#include "ClosureValue.h"
#include "UndefinedValue.h"
#include "Scope.h"

namespace Powder
{
	ClosureValue::ClosureValue() : scope(this)
	{
	}

	ClosureValue::ClosureValue(const Executable* executable, uint64_t programBufferLocation) : AddressValue(executable, programBufferLocation), scope(this)
	{
	}

	/*virtual*/ ClosureValue::~ClosureValue()
	{
	}

	/*virtual*/ Value* ClosureValue::Copy() const
	{
		return new ClosureValue(this->executable.Get(), this->programBufferLocation);
	}

	/*virtual*/ Value* ClosureValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		return new UndefinedValue();
	}
}