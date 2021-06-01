#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API JumpInstruction : public Instruction
	{
	public:
		JumpInstruction();
		virtual ~JumpInstruction();

		virtual uint8_t OpCode() const override;

		enum Type
		{
			JUMP_TO_EMBEDDED_ADDRESS,
			JUMP_TO_LOADED_ADDRESS
		};

		virtual Executor::Result Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;
	};
}