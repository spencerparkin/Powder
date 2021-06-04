#include "PopInstruction.h"
#include "Executor.h"
#include "Scope.h"
#include "Value.h"
#include "Executor.h"

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

	/*virtual*/ uint32_t PopInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		// I'm not sure where, if anywhere, this instruction is actually useful, but
		// I feel compelled to provide it, because it is the symmetric opposite to the
		// push instruction, which creates values from nowhere and then pushes them
		// onto the evaluation stack.  Here we pop a value off the evaluation stack
		// top and put it, well, nowhere.
		executor->GetCurrentScope()->PopValueFromEvaluationStackTop();
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void PopInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		programBufferLocation += 1;
	}
}