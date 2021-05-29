#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API YieldInstruction : public Instruction
	{
	public:
		YieldInstruction();
		virtual ~YieldInstruction();

		static uint8_t OpCode() { return 0x04; }

		virtual Executor::Result Execute(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
	};
}