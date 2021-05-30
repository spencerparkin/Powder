#pragma once

#include "Value.h"

namespace Powder
{
	// The typical use-case here is in storing the return
	// address needed for procedure calls.
	class POWDER_API AddressValue : public Value
	{
	public:
		AddressValue();
		AddressValue(uint64_t programBufferLocation);
		virtual ~AddressValue();

		virtual Value* Copy() const override;

		operator uint64_t() const { return this->programBufferLocation; }

	private:
		uint64_t programBufferLocation;
	};
}