#include "SysCallInstruction.h"
#include "GarbageCollector.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "NumberValue.h"
#include "StringValue.h"
#include "MapValue.h"
#include "UndefinedValue.h"
#include "VirtualMachine.h"
#include "Exceptions.hpp"
#include "Executor.h"
#include "Executable.h"
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

	/*static*/ SysCallInstruction::SysCall SysCallInstruction::TranslateAsSysCall(const std::string& funcName)
	{
		if (funcName == "exit")
			return SysCall::EXIT;
		else if (funcName == "gc")
			return SysCall::GC;
		else if (funcName == "input")
			return SysCall::INPUT;
		else if (funcName == "output")
			return SysCall::OUTPUT;
		else if (funcName == "module")
			return SysCall::MODULE;
		else if (funcName == "run")
			return SysCall::RUN_SCRIPT;

		return SysCall::UNKNOWN;
	}

	/*static*/ uint32_t SysCallInstruction::ArgumentCount(SysCall sysCall)
	{
		switch (sysCall)
		{
			case SysCall::EXIT:
				return 0;
			case SysCall::GC:
				return 0;
			case SysCall::INPUT:
				return 0;
			case SysCall::OUTPUT:
				return 1;
			case SysCall::MODULE:
				return 1;
			case SysCall::RUN_SCRIPT:
				return 1;
		}

		return -1;
	}

	/*virtual*/ uint32_t SysCallInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		const uint8_t* programBuffer = executable->byteCodeBuffer;
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
				executor->PushValueOntoEvaluationStackTop(new UndefinedValue());
				programBufferLocation += 2;
				break;
			}
			case SysCall::INPUT:
			{
				std::string str;
				std::getline(std::cin, str);

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

				executor->PushValueOntoEvaluationStackTop(value);

				programBufferLocation += 2;
				break;
			}
			case SysCall::OUTPUT:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				std::string str = value->ToString();
				std::cout << str;
				executor->PushValueOntoEvaluationStackTop(new NumberValue(str.length()));
				programBufferLocation += 2;
				break;
			}
			case SysCall::MODULE:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				std::string moduleRelativePath = value->ToString();
				std::string moduleAbsolutePath = this->ResolveModulePath(moduleRelativePath);
				MapValue* functionMapValue = virtualMachine->LoadModuleFunctionMap(moduleAbsolutePath);
				if (!functionMapValue)
					throw new RunTimeException(FormatString("Module (%s) did not generate function map value.", moduleAbsolutePath.c_str()));
				executor->PushValueOntoEvaluationStackTop(functionMapValue);
				programBufferLocation += 2;
				break;
			}
			case SysCall::RUN_SCRIPT:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				std::string scriptRelativePath = value->ToString();
				std::string scriptAbsolutePath = this->ResolveScriptPath(scriptRelativePath);
				virtualMachine->ExecuteSourceCodeFile(scriptAbsolutePath.c_str(), executor->GetCurrentScope());
				executor->PushValueOntoEvaluationStackTop(new UndefinedValue());
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

	/*virtual*/ void SysCallInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* sysCallEntry = this->assemblyData->configMap.LookupPtr("sysCall");
			if (!sysCallEntry)
				throw new CompileTimeException("System call instruction can't be assembled without knowing what system call to call.", &this->assemblyData->fileLocation);

			uint8_t* programBuffer = executable->byteCodeBuffer;
			programBuffer[programBufferLocation + 1] = sysCallEntry->code;
		}

		programBufferLocation += 2;
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string SysCallInstruction::Print(void) const
	{
		std::string detail;
		detail += "sys-call: ";
		const AssemblyData::Entry* sysCallEntry = this->assemblyData->configMap.LookupPtr("sysCall");
		if (!sysCallEntry)
			detail += "?";
		else
			detail += FormatString("%04d", sysCallEntry->code);
		return detail;
	}
#endif

	/*static*/ std::string SysCallInstruction::ResolveModulePath(const std::string& moduleRelativePath)
	{
		// TODO: Resolve the path here based on an environment variable?
		return moduleRelativePath;
	}

	/*static*/ std::string SysCallInstruction::ResolveScriptPath(const std::string& scriptRelativePath)
	{
		// TODO: Resolve path using env-vars and CWD?
		return scriptRelativePath;
	}
}