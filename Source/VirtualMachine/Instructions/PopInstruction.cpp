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

	/*virtual*/ uint32_t PopInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		GCReference<Value> value;
		executor->PopValueFromEvaluationStackTop(value);
		programBufferLocation += 1;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void PopInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		// TODO: Add byte for count of how many pops to do?
		programBufferLocation += 1;
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string PopInstruction::Print(void) const
	{
		std::string detail;
		detail += "pop";
		return detail;
	}
#endif
}