#include "AddressValue.h"
#include "NullValue.h"
#include "Executable.h"

namespace Powder
{
	AddressValue::AddressValue()
	{
		this->programBufferLocation = 0;
	}

	AddressValue::AddressValue(const Executable* executable, uint64_t programBufferLocation)
	{
		this->executableRef.Set(const_cast<Executable*>(executable));
		this->programBufferLocation = programBufferLocation;
	}

	/*virtual*/ AddressValue::~AddressValue()
	{
	}

	/*virtual*/ Value* AddressValue::Copy() const
	{
		return new AddressValue(this->executableRef.Get(), this->programBufferLocation);
	}

	/*virtual*/ Value* AddressValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		return new NullValue();
	}

	/*virtual*/ std::string AddressValue::ToString() const
	{
		return std::format("Addr: {}", int(this->programBufferLocation));
	}
}