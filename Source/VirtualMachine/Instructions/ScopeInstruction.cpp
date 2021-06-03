#include "ScopeInstruction.h"
#include "Assembler.h"
#include "Exceptions.hpp"

namespace Powder
{
	ScopeInstruction::ScopeInstruction()
	{
	}

	/*virtual*/ ScopeInstruction::~ScopeInstruction()
	{
	}

	/*virtual*/ uint8_t ScopeInstruction::OpCode() const
	{
		return 0x08;
	}

	/*virtual*/ Executor::Result ScopeInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		uint8_t scopeOp = programBuffer[programBufferLocation + 1];
		switch (scopeOp)
		{
			case ScopeOp::PUSH:
			{
				executor->PushScope();
				break;
			}
			case ScopeOp::POP:
			{
				executor->PopScope();
				break;
			}
			default:
			{
				throw new RunTimeException(FormatString("Encountered unknonwn scope operation: 0x%04x", scopeOp));
			}
		}

		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void ScopeInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			programBuffer[programBufferLocation + 1] = -1;

			const AssemblyData::Entry* scopeOpEntry = this->assemblyData->configMap.LookupPtr("scopeOp");
			if (!scopeOpEntry)
				throw new CompileTimeException("Failed to assemble scope instruction because no scope operation was specified.");
			
			programBuffer[programBufferLocation + 1] = scopeOpEntry->code;
		}

		programBufferLocation += 2;
	}
}