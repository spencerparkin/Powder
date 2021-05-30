#include "PopInstruction.h"

namespace Powder
{
	PopInstruction::PopInstruction()
	{
	}

	/*virtual*/ PopInstruction::~PopInstruction()
	{
	}

	/*virtual*/ uint8_t PopInstruction::OpCode() const
	{
		return 0x06;
	}

	/*virtual*/ Executor::Result PopInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		//...
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void PopInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
	}
}