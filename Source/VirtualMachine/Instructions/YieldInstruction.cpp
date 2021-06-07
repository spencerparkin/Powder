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

	/*virtual*/ uint32_t YieldInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		programBufferLocation++;
		return Executor::Result::YIELD;
	}

	/*virtual*/ void YieldInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		programBufferLocation++;
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string YieldInstruction::Print(void) const
	{
		std::string detail;
		detail += "yield";
		return detail;
	}
#endif
}