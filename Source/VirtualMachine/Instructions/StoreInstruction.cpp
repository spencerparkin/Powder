#include "StoreInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "Executor.h"
#include "Executable.h"
#include "VirtualMachine.h"

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

	/*virtual*/ uint32_t StoreInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		const uint8_t* programBuffer = executable->byteCodeBuffer;
		std::string name = this->ExtractEmbeddedString(programBuffer, programBufferLocation + 1);
		if (!executor->StoreAndPopValueFromEvaluationStackTop(name.c_str(), error, virtualMachine->GetDebuggerTrap()))
			return Executor::Result::RUNTIME_ERROR;
		programBufferLocation += 1 + name.length() + 1;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool StoreInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		const AssemblyData::Entry* nameEntry = this->assemblyData->configMap.LookupPtr("name");
		if (!nameEntry)
		{
			error.Add(std::string(this->assemblyData->fileLocation) + "Assembly of store instruction failed because no identifier was given.");
			return false;
		}

		if (assemblyPass == AssemblyPass::RENDER)
		{
			uint8_t* programBuffer = executable->byteCodeBuffer;
			::memcpy_s(&programBuffer[programBufferLocation + 1], nameEntry->string.length(), nameEntry->string.c_str(), nameEntry->string.length());
			programBuffer[programBufferLocation + 1 + nameEntry->string.length()] = '\0';
		}

		programBufferLocation += 1 + nameEntry->string.length() + 1;
		return true;
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string StoreInstruction::Print(void) const
	{
		std::string detail;
		detail += "store: ";
		const AssemblyData::Entry* nameEntry = this->assemblyData->configMap.LookupPtr("name");
		if (!nameEntry)
			detail += "?";
		else
			detail += nameEntry->string;
		return detail;
	}
#endif
}