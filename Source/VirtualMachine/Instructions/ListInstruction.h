#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API ListInstruction : public Instruction
	{
	public:
		ListInstruction();
		virtual ~ListInstruction();

		virtual uint8_t OpCode() const override;
		virtual uint32_t Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;

		enum Action
		{
			PUSH_LEFT,
			POP_LEFT,
			PUSH_RIGHT,
			POP_RIGHT
		};

#if defined POWDER_DEBUG
		virtual std::string Print(void) const override;
#endif
	};
}