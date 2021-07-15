#pragma once

#include "Value.h"
#include "GCSteward.hpp"

namespace Powder
{
	class Executable;

	// TODO: Maybe make a derivative of this class called ClosureValue and have
	//       it own a set of capture values.  I'm not sure how those would make
	//       it into the function scope when the call is made, but it's an idea.
	//       Maybe you could own a scope object in the closure value and then
	//       sneakily insert it into the scope chain at the call jump.
	class POWDER_API AddressValue : public Value
	{
	public:
		AddressValue();
		AddressValue(const Executable* executable, uint64_t programBufferLocation);
		virtual ~AddressValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const override;

		GCSteward<Executable> executable;
		uint64_t programBufferLocation;
	};
}