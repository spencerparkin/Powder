#pragma once

#include "Value.h"
#include "GCSteward.hpp"

namespace Powder
{
	class Executable;

	class POWDER_API AddressValue : public Value
	{
	public:
		AddressValue();
		AddressValue(const Executable* executable, uint64_t programBufferLocation);
		virtual ~AddressValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;

		GCSteward<Executable> executable;
		uint64_t programBufferLocation;
	};
}