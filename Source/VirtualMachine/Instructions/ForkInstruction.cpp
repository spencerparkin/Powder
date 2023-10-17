#include "ForkInstruction.h"
#include "VirtualMachine.h"
#include "Assembler.h"
#include "Executor.h"
#include "Executable.h"

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

	/*virtual*/ uint32_t ForkInstruction::Execute(GC::Reference<Executable, true>& executableRef, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		const uint8_t* programBuffer = executableRef.Get()->byteCodeBuffer;
		uint64_t forkedProgramBufferLocation = 0;
		::memcpy(&forkedProgramBufferLocation, &programBuffer[programBufferLocation + 1], sizeof(uint64_t));
		if (!virtualMachine->CreateExecutorAtLocation(forkedProgramBufferLocation, executor->GetExecutable(), executor->GetCurrentScope(), error))
			return Executor::Result::RUNTIME_ERROR;
		programBufferLocation += sizeof(uint8_t) + sizeof(uint64_t);
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool ForkInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* forkEntry = this->assemblyData->configMap.LookupPtr("fork");
			if (!forkEntry)
			{
				error.Add(std::string(this->assemblyData->fileLocation) + "Can't assemble fork instruction when not given the fork address information.");
				return false;
			}

			uint8_t* programBuffer = executable->byteCodeBuffer;
			::memcpy_s(&programBuffer[programBufferLocation + 1], sizeof(uint64_t), &forkEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
		}

		programBufferLocation += 1 + sizeof(uint64_t);
		return true;
	}

	/*virtual*/ std::string ForkInstruction::Print(void) const
	{
		std::string detail;
		detail += "fork: ";
		const AssemblyData::Entry* forkEntry = this->assemblyData->configMap.LookupPtr("fork");
		detail += std::format("{}", (forkEntry ? forkEntry->instruction->assemblyData->programBufferLocation : -1));
		return detail;
	}
}