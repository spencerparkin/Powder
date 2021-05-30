#include "BranchInstruction.h"

namespace Powder
{
	BranchInstruction::BranchInstruction()
	{
	}

	/*virtual*/ BranchInstruction::~BranchInstruction()
	{
	}

	/*virtual*/ uint8_t BranchInstruction::OpCode() const
	{
		return 0x01;
	}

	/*virtual*/ Executor::Result BranchInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		// TODO: Look at value on top of eval stack.  Call virtual method on value to get it's "truth state" to know if we change the program location
		//       to immediately after the branch instruction or to the given offset embedded in the instruction.  An unconditional jump should be
		//       placed where needed so that a non-jump here does cause us to fall into the else-clause, if any, of the if-statement.

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void BranchInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
	}
}