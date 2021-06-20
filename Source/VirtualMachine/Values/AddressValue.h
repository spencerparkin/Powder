#pragma once

#include "Value.h"

namespace Powder
{
	class POWDER_API AddressValue : public Value
	{
	public:
		AddressValue();
		AddressValue(uint64_t programBufferLocation);
		virtual ~AddressValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;

		operator uint64_t() const { return this->programBufferLocation; }

	private:
		uint64_t programBufferLocation;
	};
}