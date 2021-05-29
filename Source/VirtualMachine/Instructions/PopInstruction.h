#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API PopInstruction : public Instruction
	{
	public:
		PopInstruction();
		virtual ~PopInstruction();

		static uint8_t OpCode() { return 0x07; }

		virtual Executor::Result Execute(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
	};
}