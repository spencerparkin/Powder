#pragma once

#include "Executor.h"

namespace Powder
{
	class VirtualMachine;

	class POWDER_API Instruction
	{
	public:
		Instruction();
		virtual ~Instruction();

		// Note that for faster execution, implimentation of this virtual
		// method need not bounds-check their access to the given program
		// buffer.  A bound error here means there is a bug in the compiler.
		virtual Executor::Result Execute(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) = 0;
	};
}