#include "ScopeInstruction.h"
#include "Assembler.h"
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

	/*virtual*/ uint32_t ScopeInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
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
				Value* value = executor->StackTop(error);
				if (!value)
					return Executor::Result::RUNTIME_ERROR;
				ClosureValue* closureValue = dynamic_cast<ClosureValue*>(value);
				if (!closureValue)
				{
					error.Add("Bind-pop scope operation expected closure value on eval-stack top.");
					return Executor::Result::RUNTIME_ERROR;
				}
				closureValue->scopeRef.Set(executor->GetCurrentScope());
				break;
			}
			default:
			{
				error.Add(std::format("Encountered unknonwn scope operation: {}", scopeOp));
				return Executor::Result::RUNTIME_ERROR;
			}
		}

		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool ScopeInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			uint8_t* programBuffer = executable->byteCodeBuffer;
			programBuffer[programBufferLocation + 1] = -1;

			const AssemblyData::Entry* scopeOpEntry = this->assemblyData->configMap.LookupPtr("scopeOp");
			if (!scopeOpEntry)
			{
				error.Add(std::string(this->assemblyData->fileLocation) + "Failed to assemble scope instruction because no scope operation was specified.");
				return false;
			}

			programBuffer[programBufferLocation + 1] = scopeOpEntry->code;
		}

		programBufferLocation += 2;
		return true;
	}

	/*virtual*/ std::string ScopeInstruction::Print(void) const
	{
		std::string detail;
		detail += "scope: ";
		const AssemblyData::Entry* scopeOpEntry = this->assemblyData->configMap.LookupPtr("scopeOp");
		if (!scopeOpEntry)
			detail += "?";
		else if (scopeOpEntry->code == ScopeOp::PUSH)
			detail += "push";
		else if (scopeOpEntry->code == ScopeOp::POP)
			detail += "pop";
		else if (scopeOpEntry->code == ScopeOp::BIND)
			detail += "bind";
		else
			detail += "???";
		return detail;
	}
}