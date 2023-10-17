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

		virtual uint32_t Execute(GC::Reference<Executable, true>& executableRef, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error) override;
		virtual bool Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const override;
		virtual std::string Print(void) const override;
	};
}