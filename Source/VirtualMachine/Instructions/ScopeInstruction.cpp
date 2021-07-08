#include "ScopeInstruction.h"
#include "Assembler.h"
#include "Exceptions.hpp"
#include "Executor.h"
#include "Executable.h"
#include "ClosureValue.h"

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

	/*virtual*/ uint32_t ScopeInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		const uint8_t* programBuffer = executable->byteCodeBuffer;
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
			case ScopeOp::BIND:
			{
				ClosureValue* closureValue = dynamic_cast<ClosureValue*>(executor->StackTop());
				if (!closureValue)
					throw new RunTimeException("Bind-pop scope operation expected closure value on eval-stack top.");
				closureValue->scope.Set(executor->GetCurrentScope());
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

	/*virtual*/ void ScopeInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			uint8_t* programBuffer = executable->byteCodeBuffer;
			programBuffer[programBufferLocation + 1] = -1;

			const AssemblyData::Entry* scopeOpEntry = this->assemblyData->configMap.LookupPtr("scopeOp");
			if (!scopeOpEntry)
				throw new CompileTimeException("Failed to assemble scope instruction because no scope operation was specified.", &this->assemblyData->fileLocation);
			
			programBuffer[programBufferLocation + 1] = scopeOpEntry->code;
		}

		programBufferLocation += 2;
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string ScopeInstruction::Print(void) const
	{
		std::string detail;
		detail += "scope: ";
		const AssemblyData::Entry* scopeOpEntry = this->assemblyData->configMap.LookupPtr("scopeOp");
		if (!scopeOpEntry)
			detail += "?";
		else
			detail += (scopeOpEntry->code == ScopeOp::PUSH) ? "push" : "pop";
		return detail;
	}
#endif
}