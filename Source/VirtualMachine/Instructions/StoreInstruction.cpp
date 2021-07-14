#include "StoreInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "Exceptions.hpp"
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

	/*virtual*/ uint32_t StoreInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		const uint8_t* programBuffer = executable->byteCodeBuffer;
		std::string name = this->ExtractEmbeddedString(programBuffer, programBufferLocation + 1);
		executor->StoreAndPopValueFromEvaluationStackTop(name.c_str());
		if (virtualMachine->GetDebuggerTrap())
			virtualMachine->GetDebuggerTrap()->ValueStored(executor->StackTop());
		programBufferLocation += 1 + name.length() + 1;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void StoreInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* nameEntry = this->assemblyData->configMap.LookupPtr("name");
		if (!nameEntry)
			throw new CompileTimeException("Assembly of store instruction failed because no identifier was given.", &this->assemblyData->fileLocation);

		if (assemblyPass == AssemblyPass::RENDER)
		{
			uint8_t* programBuffer = executable->byteCodeBuffer;
			::memcpy_s(&programBuffer[programBufferLocation + 1], nameEntry->string.length(), nameEntry->string.c_str(), nameEntry->string.length());
			programBuffer[programBufferLocation + 1 + nameEntry->string.length()] = '\0';
		}

		programBufferLocation += 1 + nameEntry->string.length() + 1;
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