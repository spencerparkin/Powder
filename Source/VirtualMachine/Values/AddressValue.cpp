#include "AddressValue.h"
#include "NullValue.h"
#include "Executable.h"

namespace Powder
{
	AddressValue::AddressValue()
	{
		this->programBufferLocation = 0;
		this->cppReturn = false;
	}

	AddressValue::AddressValue(const Executable* executable, uint64_t programBufferLocation)
	{
		this->executableRef.Set(const_cast<Executable*>(executable));
		this->programBufferLocation = programBufferLocation;
		this->cppReturn = false;
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
		return std::format("Addr: {:#08x}", int(this->programBufferLocation));
	}

	/*virtual*/ void AddressValue::PopulateIterationArray(std::vector<Object*>& iterationArray)
	{
		iterationArray.push_back(&this->executableRef);
	}

	/*virtual*/ std::string AddressValue::GetTypeString() const
	{
		return "address";
	}

	/*virtual*/ std::string AddressValue::GetSetKey() const
	{
		return std::format("address:{}", int(this->programBufferLocation));
	}
}