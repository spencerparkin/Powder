#include "SysCallInstruction.h"
#include "GarbageCollector.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "NumberValue.h"
#include "StringValue.h"
#include "UndefinedValue.h"
#include "Exceptions.hpp"
#include "Executor.h"
#include <iostream>

namespace Powder
{
	SysCallInstruction::SysCallInstruction()
	{
	}

	/*virtual*/ SysCallInstruction::~SysCallInstruction()
	{
	}

	/*virtual*/ uint8_t SysCallInstruction::OpCode() const
	{
		return 0x0A;
	}

	/*virtual*/ uint32_t SysCallInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		uint8_t sysCallCode = programBuffer[programBufferLocation + 1];
		switch (sysCallCode)
		{
			case SysCall::EXIT:
			{
				return Executor::Result::HALT;
			}
			case SysCall::GC:
			{
				GarbageCollector::GC()->FullPass();
				programBufferLocation += 2;
				break;
			}
			case SysCall::INPUT:
			{
				std::string str;
				std::cin >> str;

				Value* value = nullptr;

				// Note that we don't need to free these values,
				// because their memory is managed by the GC.
				value = new NumberValue();
				if (!value->FromString(str))
				{
					value = new StringValue();
					if (!value->FromString(str))
					{
						value = new UndefinedValue();
					}
				}

				executor->GetCurrentScope()->PushValueOntoEvaluationStackTop(value);

				programBufferLocation += 2;
				break;
			}
			case SysCall::OUTPUT:
			{
				Value* value = executor->GetCurrentScope()->PopValueFromEvaluationStackTop();
				std::cout << value->ToString() << std::endl;
				programBufferLocation += 2;
				break;
			}
			default:
			{
				throw new RunTimeException(FormatString("Encountered unknown system call: 0x%04x", sysCallCode));
			}
		}

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void SysCallInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* sysCallEntry = this->assemblyData->configMap.LookupPtr("sysCall");
			if (!sysCallEntry)
				throw new CompileTimeException("System call instruction can't be assembled without knowing what system call to call.");

			programBuffer[programBufferLocation + 1] = sysCallEntry->code;
		}

		programBufferLocation += 2;
	}
}