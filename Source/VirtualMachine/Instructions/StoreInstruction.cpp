#include "StoreInstruction.h"

namespace Powder
{
	StoreInstruction::StoreInstruction()
	{
	}

	/*virtual*/ StoreInstruction::~StoreInstruction()
	{
	}

	/*virtual*/ uint8_t StoreInstruction::OpCode() const
	{
		return 0x09;
	}

	/*virtual*/ Executor::Result StoreInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		// TODO: Call executor->StoreValue() to move value form evaluation stack to scope.  We pop the stack in the process.

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void StoreInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
	}
}