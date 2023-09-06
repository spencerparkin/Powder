#pragma once

#include "AddressValue.h"
#include "Reference.h"

namespace Powder
{
	class Scope;

	class POWDER_API ClosureValue : public AddressValue
	{
	public:
		ClosureValue();
		ClosureValue(const Executable* executable, uint64_t programBufferLocation);
		virtual ~ClosureValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;

		GC::Reference<Scope, false> scopeRef;
	};
}