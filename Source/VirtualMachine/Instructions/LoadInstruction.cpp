#include "LoadInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "Exceptions.hpp"

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

	/*virtual*/ Executor::Result LoadInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		std::string name = this->ExtractEmbeddedString(programBuffer, programBufferLocation + 1);
		executor->GetCurrentScope()->LoadAndPushValueOntoEvaluationStackTop(name.c_str());
		programBufferLocation += 1 + name.length() + 1;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void LoadInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* nameEntry = this->assemblyData->configMap.LookupPtr("name");
		if (!nameEntry)
			throw new CompileTimeException("Can't assemble load instruction if not given identifier information.");

		if (assemblyPass == AssemblyPass::RENDER)
		{
			::memcpy_s(&programBuffer[programBufferLocation + 1], nameEntry->string.length(), nameEntry->string.c_str(), nameEntry->string.length());
			programBuffer[programBufferLocation + 1 + nameEntry->string.length()] = '\0';
		}

		programBufferLocation += 1 + nameEntry->string.length() + 1;
	}
}