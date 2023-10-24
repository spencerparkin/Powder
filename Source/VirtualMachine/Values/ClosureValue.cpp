#include "ClosureValue.h"
#include "Executable.h"
#include "NullValue.h"
#include "Scope.h"

namespace Powder
{
	ClosureValue::ClosureValue()
	{
	}

	ClosureValue::ClosureValue(const Executable* executable, uint64_t programBufferLocation) : AddressValue(executable, programBufferLocation)
	{
	}

	/*virtual*/ ClosureValue::~ClosureValue()
	{
	}

	/*virtual*/ Value* ClosureValue::Copy() const
	{
		return new ClosureValue(this->executableRef.Get(), this->programBufferLocation);
	}

	/*virtual*/ Value* ClosureValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		return new NullValue();
	}

	/*virtual*/ void ClosureValue::PopulateIterationArray(std::vector<Object*>& iterationArray)
	{
		AddressValue::PopulateIterationArray(iterationArray);
		iterationArray.push_back(&this->scopeRef);
	}

	/*virtual*/ std::string ClosureValue::GetTypeString() const
	{
		return "closure";
	}

	/*virtual*/ std::string ClosureValue::GetSetKey() const
	{
		return std::format("closure:{}|", uintptr_t(this->scopeRef.Get())) + AddressValue::GetSetKey();
	}
}