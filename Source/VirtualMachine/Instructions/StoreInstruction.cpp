#include "StoreInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "Exceptions.hpp"
#include "Executor.h"

namespace Powder
{
	StoreInstruction::StoreInstruction()
	{
	}

	/*virtual*/ StoreInstruction::~StoreInstruction()
	{
	}

	/*virtual*/ uint8_t StoreInstruction::OpCode() const
	{
		return 0x09;
	}

	/*virtual*/ uint32_t StoreInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		std::string name = this->ExtractEmbeddedString(programBuffer, programBufferLocation + 1);
		executor->StoreAndPopValueFromEvaluationStackTop(name.c_str());
		programBufferLocation += 1 + name.length() + 1;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void StoreInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* nameEntry = this->assemblyData->configMap.LookupPtr("name");
		if (!nameEntry)
			throw new CompileTimeException("Assembly of store instruction failed because no identifier was give.");

		if (assemblyPass == AssemblyPass::RENDER)
		{
			::memcpy_s(&programBuffer[programBufferLocation + 1], nameEntry->string.length(), nameEntry->string.c_str(), nameEntry->string.length());
			programBuffer[programBufferLocation + 1 + nameEntry->string.length()] = '\0';
		}

		programBufferLocation += 1 + nameEntry->string.length() + 1;
	}
}