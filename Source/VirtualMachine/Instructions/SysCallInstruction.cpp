#include "SysCallInstruction.h"
#include "GarbageCollector.h"

namespace Powder
{
	SysCallInstruction::SysCallInstruction()
	{
	}

	/*virtual*/ SysCallInstruction::~SysCallInstruction()
	{
	}

	/*virtual*/ uint8_t SysCallInstruction::OpCode() const
	{
		return 0x0A;
	}

	/*virtual*/ Executor::Result SysCallInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		uint8_t sysCallCode = programBuffer[programBufferLocation + 1];
		switch (sysCallCode)
		{
			case int(SysCall::EXIT):
			{
				return Executor::Result::HALT;
			}
			case int(SysCall::GC):
			{
				// Rather than having the VM periodically running the GC during execution, I'm leaving
				// this as something the program has to do manually until I can find a better GC algorithm.
				// Even if the program never calls this, it should not leak memory as a full pass is made
				// over all objects when the VM finishes execution of a given program.
				GarbageCollector::GC()->FullPass();
				break;
			}
			case int(SysCall::INPUT):
			{
				// TODO: Block for user input, then load result onto the evaluation stack.
				break;
			}
			case int(SysCall::OUTPUT) :
			{
				// TODO: Take top of eval stack and print it to the console.
				break;
			}
		}

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void SysCallInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
	}
}