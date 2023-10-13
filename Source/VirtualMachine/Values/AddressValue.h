#pragma once

#include "Value.h"
#include "Reference.h"

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
		virtual std::string ToString() const override;
		virtual void PopulateIterationArray(std::vector<Object*>& iterationArray) override;
		virtual std::string GetTypeString() const override;

		GC::Reference<Executable, false> executableRef;
		uint64_t programBufferLocation;
	};
}