#include "PushInstruction.h"

namespace Powder
{
	PushInstruction::PushInstruction()
	{
	}

	/*virtual*/ PushInstruction::~PushInstruction()
	{
	}

	/*virtual*/ Executor::Result PushInstruction::Execute(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		// TODO: Call executor->LoadValue() function with value that is eiher an immediate value found in the given buffer,
		//       or with a value from scope by the name found in the given buffer.  The next byte will tell us which style
		//       of the instruction we're doing.  We might also support here executor->PushScope().

		return Executor::Result::CONTINUE;
	}
}