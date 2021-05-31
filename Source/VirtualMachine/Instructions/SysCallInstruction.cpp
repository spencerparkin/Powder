#include "SysCallInstruction.h"
#include "GarbageCollector.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "NumberValue.h"
#include "StringValue.h"
#include "UndefinedValue.h"
#include "Exceptions.hpp"
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

	/*virtual*/ Executor::Result SysCallInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
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
				break;
			}
			case SysCall::OUTPUT:
			{
				Value* value = executor->GetCurrentScope()->PopValueFromEvaluationStackTop();
				std::cout << value->ToString() << std::endl;
				break;
			}
			default:
			{
				throw new RunTimeException("Encountered unknown system call 0x%04x");
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
			{
				// TODO: Throw an exception.
			}

			programBuffer[programBufferLocation + 1] = sysCallEntry->code;
		}

		programBufferLocation += 2;
	}
}