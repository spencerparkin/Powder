#include "LoadInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "Executor.h"
#include "Executable.h"
#include "VirtualMachine.h"

namespace Powder
{
	LoadInstruction::LoadInstruction()
	{
	}

	/*virtual*/ LoadInstruction::~LoadInstruction()
	{
	}

	/*virtual*/ uint8_t LoadInstruction::OpCode() const
	{
		return 0x04;
	}

	/*virtual*/ uint32_t LoadInstruction::Execute(GC::Reference<Executable, false>& executableRef, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		const uint8_t* programBuffer = executableRef.Get()->byteCodeBuffer;
		std::string name = this->ExtractEmbeddedString(programBuffer, programBufferLocation + 1);
		if (!executor->LoadAndPushValueOntoEvaluationStackTop(name.c_str(), error, virtualMachine->GetDebuggerTrap()))
			return Executor::Result::RUNTIME_ERROR;
		programBufferLocation += 1 + name.length() + 1;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool LoadInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		const AssemblyData::Entry* nameEntry = this->assemblyData->configMap.LookupPtr("name");
		if (!nameEntry)
		{
			error.Add(std::string(this->assemblyData->fileLocation) + "Can't assemble load instruction if not given identifier information.");
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

	/*virtual*/ std::string LoadInstruction::Print(void) const
	{
		std::string detail;
		detail += "load: ";
		const AssemblyData::Entry* nameEntry = this->assemblyData->configMap.LookupPtr("name");
		detail += nameEntry ? nameEntry->string : "?";
		return detail;
	}
}