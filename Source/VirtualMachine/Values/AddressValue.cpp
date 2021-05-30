#include "AddressValue.h"

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
}