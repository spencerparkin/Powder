#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API LoadInstruction : public Instruction
	{
	public:
		LoadInstruction();
		virtual ~LoadInstruction();

		static uint8_t OpCode() { return 0x09; }

		virtual Executor::Result Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;
	};
}