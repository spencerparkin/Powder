#pragma once

#include "Executor.h"

namespace Powder
{
	class VirtualMachine;
	class Stream;
	class AssemblyData;

	class POWDER_API Instruction
	{
	public:
		Instruction();
		virtual ~Instruction();

		// Note that for faster execution, implimentation of this virtual
		// method need not bounds-check their access to the given program
		// buffer.  A bound error here means there is a bug in the compiler.
		virtual Executor::Result Execute(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) = 0;

		enum class AssemblyPass
		{
			// In this pass, all instructions get layed down into the
			// executable program buffer, and are thereby located.
			LOCATE,

			// In this pass, all instructions that refer to other instructions
			// by pointer can now be resolved as refering to them by location.
			// This completes the assembly of, e.g., unconditional jump instructions
			// to fixed locations in the program buffer.
			LINK
		};

		//virtual void Assemble(Stream* stream, AssemblyPass assemblyPass) = 0;

	protected:

		AssemblyData* assemblyData;
	};
}