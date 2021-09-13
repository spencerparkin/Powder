#include "BranchInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "Exceptions.hpp"
#include "Executor.h"
#include "Executable.h"

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

	/*virtual*/ uint32_t BranchInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		const uint8_t* programBuffer = executable->byteCodeBuffer;
		Value* value = executor->PopValueFromEvaluationStackTop(true);
		if (value->AsBoolean())
			programBufferLocation += 1 + sizeof(uint64_t);
		else
			::memcpy_s(&programBufferLocation, sizeof(uint64_t), &programBuffer[programBufferLocation + 1], sizeof(uint64_t));
		value->DecRef();
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void BranchInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* branchEntry = this->assemblyData->configMap.LookupPtr("branch");
			if (!branchEntry)
				throw new CompileTimeException("Cannot assemble branch instruction without the branch address information being given.", &this->assemblyData->fileLocation);

			uint8_t* programBuffer = executable->byteCodeBuffer;
			::memcpy_s(&programBuffer[programBufferLocation + 1], sizeof(uint64_t), &branchEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
		}

		programBufferLocation += 1 + sizeof(uint64_t);
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string BranchInstruction::Print(void) const
	{
		std::string detail;
		detail += "branch: ";
		const AssemblyData::Entry* branchEntry = this->assemblyData->configMap.LookupPtr("branch");
		detail += FormatString("%04d", (branchEntry ? branchEntry->instruction->assemblyData->programBufferLocation : -1));
		return detail;
	}
#endif
}