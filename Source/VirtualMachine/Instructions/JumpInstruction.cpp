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

	/*virtual*/ Executor::Result JumpInstruction::Execute(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		::memcpy(&programBufferLocation, &programBuffer[programBufferLocation + 1], sizeof(uint64_t));
		return Executor::Result::CONTINUE;
	}
}