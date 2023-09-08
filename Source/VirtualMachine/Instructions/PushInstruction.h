#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API PushInstruction : public Instruction
	{
	public:
		PushInstruction();
		virtual ~PushInstruction();

		virtual uint8_t OpCode() const override;

		enum DataType
		{
			UNDEFINED,
			NUMBER,
			STRING,
			EMPTY_LIST,
			EMPTY_MAP,
			ADDRESS,
			CLOSURE,
			EXISTING_VALUE
		};

		virtual uint32_t Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error) override;
		virtual bool Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const override;

#if defined POWDER_DEBUG
		virtual std::string Print(void) const override;
#endif
	};
}