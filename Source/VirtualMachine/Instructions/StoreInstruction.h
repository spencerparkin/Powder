#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API StoreInstruction : public Instruction
	{
	public:
		StoreInstruction();
		virtual ~StoreInstruction();

		virtual uint8_t OpCode() const override;
		virtual Executor::Result Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;
	};
}