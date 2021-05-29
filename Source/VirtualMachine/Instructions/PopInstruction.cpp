#include "PopInstruction.h"

namespace Powder
{
	PopInstruction::PopInstruction()
	{
	}

	/*virtual*/ PopInstruction::~PopInstruction()
	{
	}

	/*virtual*/ Executor::Result PopInstruction::Execute(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		// TODO: Call executor->StoreValue().  Also call executor->PopScope().

		return Executor::Result::CONTINUE;
	}
}