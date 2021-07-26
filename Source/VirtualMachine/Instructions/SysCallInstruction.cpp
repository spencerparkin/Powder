#include "SysCallInstruction.h"
#include "GarbageCollector.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "NumberValue.h"
#include "StringValue.h"
#include "ContainerValue.h"
#include "CppFunctionValue.h"
#include "MapValue.h"
#include "UndefinedValue.h"
#include "VirtualMachine.h"
#include "Exceptions.hpp"
#include "Executor.h"
#include "Executable.h"
#include "PathResolver.h"
#include <iostream>
#include <Windows.h>

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
		else if (funcName == "gc_count")
			return SysCall::GC_COUNT;
		else if (funcName == "input")
			return SysCall::INPUT;
		else if (funcName == "output")
			return SysCall::OUTPUT;
		else if (funcName == "module")
			return SysCall::MODULE;
		else if (funcName == "run")
			return SysCall::RUN_SCRIPT;
		else if (funcName == "sleep")
			return SysCall::SLEEP;
		else if (funcName == "iterator")
			return SysCall::ITERATOR;
		else if (funcName == "as_str")
			return SysCall::AS_STRING;
		else if (funcName == "as_num")
			return SysCall::AS_NUMBER;

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
			case SysCall::GC_COUNT:
				return 0;
			case SysCall::INPUT:
				return 0;
			case SysCall::OUTPUT:
				return 1;
			case SysCall::MODULE:
				return 1;
			case SysCall::RUN_SCRIPT:
				return 1;
			case SysCall::SLEEP:
				return 1;
			case SysCall::ITERATOR:
				return 1;
			case SysCall::AS_STRING:
				return 1;
			case SysCall::AS_NUMBER:
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
				GarbageCollector::GC()->FullPurge();
				executor->PushValueOntoEvaluationStackTop(new UndefinedValue());
				break;
			}
			case SysCall::INPUT:
			{
				std::string str;
				virtualMachine->GetIODevice()->InputString(str);

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
				break;
			}
			case SysCall::OUTPUT:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				std::string str = value->ToString();
				virtualMachine->GetIODevice()->OutputString(str);
				executor->PushValueOntoEvaluationStackTop(new NumberValue(str.length()));
				break;
			}
			case SysCall::MODULE:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				std::string moduleRelativePath = value->ToString();
				std::string moduleAbsolutePath = pathResolver.ResolvePath(moduleRelativePath, PathResolver::SEARCH_BASE | PathResolver::SEARCH_CWD);
				MapValue* functionMapValue = virtualMachine->LoadModuleFunctionMap(moduleAbsolutePath);
				if (!functionMapValue)
					throw new RunTimeException(FormatString("Module (%s) did not generate function map value.", moduleAbsolutePath.c_str()));
				executor->PushValueOntoEvaluationStackTop(functionMapValue);
				break;
			}
			case SysCall::RUN_SCRIPT:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				std::string scriptRelativePath = value->ToString();
				std::string scriptAbsolutePath = pathResolver.ResolvePath(scriptRelativePath, PathResolver::SEARCH_CWD);
				virtualMachine->ExecuteSourceCodeFile(scriptAbsolutePath.c_str(), executor->GetCurrentScope());
				executor->PushValueOntoEvaluationStackTop(new UndefinedValue());
				break;
			}
			case SysCall::SLEEP:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				double sleepSeconds = value->AsNumber();
				if (sleepSeconds > 0.0)
					::Sleep(DWORD(sleepSeconds * 1000.0f));
				executor->PushValueOntoEvaluationStackTop(new UndefinedValue());
				break;
			}
			case SysCall::GC_COUNT:
			{
				uint32_t count = GarbageCollector::GC()->HonestCollectableCount();
				NumberValue* numberValue = new NumberValue(count);
				executor->PushValueOntoEvaluationStackTop(numberValue);
				break;
			}
			case SysCall::ITERATOR:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				ContainerValue* containerValue = dynamic_cast<ContainerValue*>(value);
				if (!containerValue)
					throw new RunTimeException("Cannot create iterator for non-container value.");
				CppFunctionValue* iteratorValue = containerValue->MakeIterator();
				if (!iteratorValue)
					throw new RunTimeException("Failed to create iterator for container value.");
				executor->PushValueOntoEvaluationStackTop(iteratorValue);
				break;
			}
			case SysCall::AS_STRING:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				StringValue* stringValue = new StringValue(value->ToString());
				executor->PushValueOntoEvaluationStackTop(stringValue);
				break;
			}
			case SysCall::AS_NUMBER:
			{
				Value* value = executor->PopValueFromEvaluationStackTop();
				NumberValue* numberValue = new NumberValue(value->AsNumber());
				executor->PushValueOntoEvaluationStackTop(numberValue);
				break;
			}
			default:
			{
				throw new RunTimeException(FormatString("Encountered unknown system call: 0x%04x", sysCallCode));
			}
		}

		programBufferLocation += 2;
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
}