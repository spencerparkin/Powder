#include "ForkInstruction.h"
#include "VirtualMachine.h"
#include "Assembler.h"
#include "Exceptions.hpp"

namespace Powder
{
	ForkInstruction::ForkInstruction()
	{
	}

	/*virtual*/ ForkInstruction::~ForkInstruction()
	{
	}

	/*virtual*/ uint8_t ForkInstruction::OpCode() const
	{
		return 0x02;
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
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* forkEntry = this->assemblyData->configMap.LookupPtr("fork");
			if (!forkEntry)
				throw new CompileTimeException("Can't assemble fork instruction when not given the fork address information.");

			::memcpy_s(&programBuffer[programBufferLocation + 1], sizeof(uint64_t), &forkEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
		}

		programBufferLocation += 1 + sizeof(uint64_t);
	}
}