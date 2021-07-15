#include "AddressValue.h"
#include "UndefinedValue.h"
#include "Executable.h"
#include "StringFormat.h"

namespace Powder
{
	AddressValue::AddressValue() : executable(this)
	{
		this->programBufferLocation = 0;
	}

	AddressValue::AddressValue(const Executable* executable, uint64_t programBufferLocation) : executable(this)
	{
		this->executable.Set(const_cast<Executable*>(executable));
		this->programBufferLocation = programBufferLocation;
	}

	/*virtual*/ AddressValue::~AddressValue()
	{
	}

	/*virtual*/ Value* AddressValue::Copy() const
	{
		return new AddressValue(this->executable.Get(), this->programBufferLocation);
	}

	/*virtual*/ Value* AddressValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		return new UndefinedValue();
	}

	/*virtual*/ std::string AddressValue::ToString() const
	{
		return FormatString("Addr: 0x%08x", int(this->programBufferLocation));
	}
}