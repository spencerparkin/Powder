#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API MapInstruction : public Instruction
	{
	public:
		MapInstruction();
		virtual ~MapInstruction();

		virtual uint8_t OpCode() const override;
		virtual uint32_t Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;

		enum Action
		{
			INSERT,
			REMOVE,
			MAKE_KEY_LIST
		};

#if defined POWDER_DEBUG
		virtual std::string Print(void) const override;
#endif
	};
}