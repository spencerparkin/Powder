#include "BranchInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "Exceptions.hpp"
#include "Executor.h"

namespace Powder
{
	BranchInstruction::BranchInstruction()
	{
	}

	/*virtual*/ BranchInstruction::~BranchInstruction()
	{
	}

	/*virtual*/ uint8_t BranchInstruction::OpCode() const
	{
		return 0x01;
	}

	/*virtual*/ uint32_t BranchInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		Value* value = executor->GetCurrentScope()->PopValueFromEvaluationStackTop();
		if (value->AsBoolean())
			programBufferLocation += 1 + sizeof(uint64_t);
		else
			::memcpy_s(&programBufferLocation, sizeof(uint64_t), &programBuffer[programBufferLocation + 1], sizeof(uint64_t));
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void BranchInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* branchEntry = this->assemblyData->configMap.LookupPtr("branch");
			if (!branchEntry)
				throw new CompileTimeException("Cannot assemble branch instruction without the branch address information being given.");

			::memcpy_s(&programBuffer[programBufferLocation + 1], sizeof(uint64_t), &branchEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
		}

		programBufferLocation += 1 + sizeof(uint64_t);
	}
}