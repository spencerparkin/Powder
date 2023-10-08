#include "YieldInstruction.h"
#include "Executor.h"

namespace Powder
{
	YieldInstruction::YieldInstruction()
	{
	}

	/*virtual*/ YieldInstruction::~YieldInstruction()
	{
	}

	/*virtual*/ uint8_t YieldInstruction::OpCode() const
	{
		return 0x0B;
	}

	/*virtual*/ uint32_t YieldInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		programBufferLocation++;
		return Executor::Result::YIELD;
	}

	/*virtual*/ bool YieldInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		programBufferLocation++;
		return true;
	}

	/*virtual*/ std::string YieldInstruction::Print(void) const
	{
		std::string detail;
		detail += "yield";
		return detail;
	}
}