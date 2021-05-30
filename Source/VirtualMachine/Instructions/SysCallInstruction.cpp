#include "SysCallInstruction.h"
#include "GarbageCollector.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "NumberValue.h"
#include "StringValue.h"
#include "UndefinedValue.h"
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
			case uint8_t(SysCall::EXIT):
			{
				return Executor::Result::HALT;
			}
			case uint8_t(SysCall::GC):
			{
				GarbageCollector::GC()->FullPass();
				break;
			}
			case uint8_t(SysCall::INPUT):
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
			case uint8_t(SysCall::OUTPUT) :
			{
				Value* value = nullptr;
				executor->GetCurrentScope()->PopValueFromEvaluationStackTop(value);
				if (value)
					std::cout << value->ToString() << std::endl;
				else
				{
					// TODO: Throw an exception.
				}
				break;
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

			if (sysCallEntry->string == "exit")
				programBuffer[programBufferLocation + 1] = uint8_t(SysCall::EXIT);
			else if (sysCallEntry->string == "gc")
				programBuffer[programBufferLocation + 1] = uint8_t(SysCall::GC);
			else if (sysCallEntry->string == "input")
				programBuffer[programBufferLocation + 1] = uint8_t(SysCall::INPUT);
			else if (sysCallEntry->string == "output")
				programBuffer[programBufferLocation + 1] = uint8_t(SysCall::OUTPUT);
		}

		programBufferLocation += 2;
	}
}