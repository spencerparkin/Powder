#include "JumpInstruction.h"
#include <string.h>

namespace Powder
{
	JumpInstruction::JumpInstruction()
	{
	}

	/*virtual*/ JumpInstruction::~JumpInstruction()
	{
	}

	/*virtual*/ uint8_t JumpInstruction::OpCode() const
	{
		return 0x03;
	}

	/*virtual*/ Executor::Result JumpInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		// TODO: We should also be able to be configured (using a config byte)
		//       to jump to an offset determined by a value in the scope.
		//       This is a run-time determined jump location, which is used primarly
		//       as the mechanism for returning from a function call.
		//       1) Push scope.
		//       2) Load arguments into scope.
		//       3) Load return address (location) into scope.
		//       4) Unconditional jump to compile-time determined offset to subroutine.
		//       5) Subroutine runs.
		//       6) Unconditional jump to return address stored in scope.
		::memcpy(&programBufferLocation, &programBuffer[programBufferLocation + 1], sizeof(uint64_t));
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void JumpInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
	}
}