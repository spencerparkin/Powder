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
			POP,
			BIND
		};

		virtual uint32_t Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;

#if defined POWDER_DEBUG
		virtual std::string Print(void) const override;
#endif
	};
}