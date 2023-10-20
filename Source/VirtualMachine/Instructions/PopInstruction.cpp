#include "PopInstruction.h"
#include "Executor.h"
#include "Scope.h"
#include "Value.h"
#include "Executor.h"

namespace Powder
{
	PopInstruction::PopInstruction()
	{
	}

	/*virtual*/ PopInstruction::~PopInstruction()
	{
	}

	/*virtual*/ uint8_t PopInstruction::OpCode() const
	{
		return 0x06;
	}

	/*virtual*/ uint32_t PopInstruction::Execute(GC::Reference<Executable, false>& executableRef, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		GC::Reference<Value, true> valueRef;
		if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
			return Executor::Result::RUNTIME_ERROR;
		programBufferLocation += 1;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool PopInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		// TODO: Add byte for count of how many pops to do?
		programBufferLocation += 1;
		return true;
	}

	/*virtual*/ std::string PopInstruction::Print(void) const
	{
		std::string detail;
		detail += "pop";
		return detail;
	}
}