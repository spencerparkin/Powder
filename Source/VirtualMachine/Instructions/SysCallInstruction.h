#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API SysCallInstruction : public Instruction
	{
	public:
		SysCallInstruction();
		virtual ~SysCallInstruction();

		virtual uint8_t OpCode() const override;

		enum SysCall
		{
			UNKNOWN,
			EXIT,
			GC,
			INPUT,
			OUTPUT
		};

		static SysCall TranslateAsSysCall(const std::string& funcName);
		static uint32_t ArgumentCount(SysCall sysCall);

		virtual uint32_t Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;
	};
}