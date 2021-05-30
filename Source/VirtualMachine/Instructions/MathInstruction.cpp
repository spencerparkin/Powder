#include "MathInstruction.h"

namespace Powder
{
	MathInstruction::MathInstruction()
	{
	}

	/*virtual*/ MathInstruction::~MathInstruction()
	{
	}

	/*virtual*/ Executor::Result MathInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		// TODO: We first decpypher here our operation from the program buffer using the next byte.
		//       We then, based on how we're configured (the next byte in the program buffer), collect our arguments
		//       as either immediates from the program buffer or as values popped from the evaluation stack.
		//       (There should be instructions for pushing values found in scope onto the evaluation stack, and
		//       instructions for popping values from the evaluation stack back into scope.)
		//       In either case, or combination of cases, we perform our operation, then push the result onto the evaluation stack.
		//       Note that this class will handle as many binary operations as possible, math or otherwise, so as to prevent
		//       an over-proliferation of C++ class types.  Or we can deligate math operations to a Value class virtual method.

		return Executor::Result::YIELD;
	}

	/*virtual*/ void MathInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
	}
}