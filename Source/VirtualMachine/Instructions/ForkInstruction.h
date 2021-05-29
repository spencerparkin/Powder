#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API ForkInstruction : public Instruction
	{
	public:
		ForkInstruction();
		virtual ~ForkInstruction();

		static uint8_t OpCode() { return 0x01; }

		virtual Executor::Result Execute(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
	};
}