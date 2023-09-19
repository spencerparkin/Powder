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
			GC_COUNT,
			INPUT,
			OUTPUT,
			MODULE,
			RUN_SCRIPT,
			SLEEP,
			AS_ITERATOR,
			AS_STRING,
			AS_NUMBER,
			RAND_INT,
			RAND_FLOAT,
			RAND_SEED,
			COS,
			SIN,
			TAN
		};

		static SysCall TranslateAsSysCall(const std::string& funcName);
		static uint32_t ArgumentCount(SysCall sysCall);

		virtual uint32_t Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error) override;
		virtual bool Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const override;

#if defined POWDER_DEBUG
		virtual std::string Print(void) const override;
#endif
	};
}