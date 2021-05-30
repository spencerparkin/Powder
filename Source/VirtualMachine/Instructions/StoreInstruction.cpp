#include "StoreInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"

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

	/*virtual*/ Executor::Result StoreInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		std::string name;
		uint64_t i = 1;
		while (programBuffer[i] != '\0')
			name += programBuffer[i];

		Value* value = nullptr;
		executor->GetCurrentScope()->PopValueFromEvaluationStackTop(value);
		executor->GetCurrentScope()->StoreValue(name.c_str(), value);

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void StoreInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
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