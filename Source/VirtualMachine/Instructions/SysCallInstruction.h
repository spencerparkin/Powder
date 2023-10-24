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
			TAN,
			LOG,
			EXP,
			SQRT,
			TYPE,
			SAME,
			ADD_MEMBER,
			DEL_MEMBER,
			ANY_MEMBER,
			ERROR_,
			RANGE
		};

		static SysCall TranslateAsSysCall(const std::string& funcName);
		static uint32_t ArgumentCount(SysCall sysCall);

		virtual uint32_t Execute(GC::Reference<Executable, false>& executableRef, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error) override;
		virtual bool Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const override;
		virtual std::string Print(void) const override;

	private:
		std::set<std::string>* runSet;
	};
}