#include "BranchInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "Executor.h"
#include "Executable.h"
#include "Reference.h"

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

	/*virtual*/ uint32_t BranchInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		const uint8_t* programBuffer = executable->byteCodeBuffer;
		GC::Reference<Value, true> valueRef;
		if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
			return Executor::Result::RUNTIME_ERROR;
		if (valueRef.Get()->AsBoolean())
			programBufferLocation += 1 + sizeof(uint64_t);
		else
			::memcpy_s(&programBufferLocation, sizeof(uint64_t), &programBuffer[programBufferLocation + 1], sizeof(uint64_t));
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool BranchInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* branchEntry = this->assemblyData->configMap.LookupPtr("branch");
			if (!branchEntry)
			{
				error.Add(std::string(this->assemblyData->fileLocation) + "Cannot assemble branch instruction without the branch address information being given.");
				return false;
			}

			uint8_t* programBuffer = executable->byteCodeBuffer;
			::memcpy_s(&programBuffer[programBufferLocation + 1], sizeof(uint64_t), &branchEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
		}

		programBufferLocation += 1 + sizeof(uint64_t);
		return true;
	}

	/*virtual*/ std::string BranchInstruction::Print(void) const
	{
		std::string detail;
		detail += "branch: ";
		const AssemblyData::Entry* branchEntry = this->assemblyData->configMap.LookupPtr("branch");
		detail += std::format("{:#08x}", (branchEntry ? branchEntry->instruction->assemblyData->programBufferLocation : -1));
		return detail;
	}
}