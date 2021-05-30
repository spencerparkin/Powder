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
		// TODO: Call executor->StoreValue() to move value from evaluation stack to scope.  We pop the stack in the process.
		//       Note that encoded in the instruction is also the scope level desired.  A scope level of 1 lets us do return values.
		//       Before jumping to the return address, a store value instruction can be used to put the return value in the outer
		//       scope by the name of "__return_value__".  The return address would then be at an instruction that loads this value
		//       onto the evaluation stack, if need be the case (e.g., in the middle of evaluating a binary expression tree.)
		//       Note that the first instruction executed after returning from a function call should probably always be a pop-scope instruction.

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void StoreInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
	}
}