#include "YieldInstruction.h"

namespace Powder
{
	YieldInstruction::YieldInstruction()
	{
	}

	/*virtual*/ YieldInstruction::~YieldInstruction()
	{
	}

	/*virtual*/ Executor::Result YieldInstruction::Execute(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		programBufferLocation++;
		return Executor::Result::YIELD;
	}
}