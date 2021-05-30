#include "ForkInstruction.h"
#include "VirtualMachine.h"
#include <string.h>

namespace Powder
{
	ForkInstruction::ForkInstruction()
	{
	}

	/*virtual*/ ForkInstruction::~ForkInstruction()
	{
	}

	/*virtual*/ Executor::Result ForkInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		uint64_t forkedProgramBufferLocation = 0;
		::memcpy(&forkedProgramBufferLocation, &programBuffer[programBufferSize + 1], sizeof(uint64_t));
		virtualMachine->CreateExecutorAtLocation(forkedProgramBufferLocation);
		programBufferLocation += sizeof(uint8_t) + sizeof(uint64_t);
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void ForkInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
	}
}