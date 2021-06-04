#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API ScopeInstruction : public Instruction
	{
	public:
		ScopeInstruction();
		virtual ~ScopeInstruction();

		virtual uint8_t OpCode() const override;

		enum ScopeOp
		{
			PUSH,
			POP
		};

		virtual uint32_t Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;
	};
}