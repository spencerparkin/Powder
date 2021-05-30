#include "LoadInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"

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
		std::string name;
		uint64_t i = 1;
		while (programBuffer[i] != '\0')
			name += programBuffer[i];

		Value* value = executor->GetCurrentScope()->LookupValue(name.c_str(), false);
		executor->GetCurrentScope()->PushValueOntoEvaluationStackTop(value);

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void LoadInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* nameEntry = this->assemblyData->configMap.LookupPtr("name");
		if (!nameEntry)
		{
			// TODO: Throw an exception.
		}

		if (assemblyPass == AssemblyPass::RENDER)
		{
			::memcpy_s(&programBuffer[programBufferLocation + 1], nameEntry->string.length(), nameEntry->string.c_str(), nameEntry->string.length());
			programBuffer[programBufferLocation + 1 + nameEntry->string.length()] = '\0';
		}

		programBufferLocation += 1 + nameEntry->string.length() + 1;
	}
}