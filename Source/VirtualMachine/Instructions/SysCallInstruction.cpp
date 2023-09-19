#include "SysCallInstruction.h"
#include "GarbageCollector.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "NumberValue.h"
#include "StringValue.h"
#include "ContainerValue.h"
#include "CppFunctionValue.h"
#include "AddressValue.h"
#include "MapValue.h"
#include "NullValue.h"
#include "VirtualMachine.h"
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
		else if (funcName == "as_iter")
			return SysCall::AS_ITERATOR;
		else if (funcName == "as_str")
			return SysCall::AS_STRING;
		else if (funcName == "as_num")
			return SysCall::AS_NUMBER;
		else if (funcName == "cos")
			return SysCall::COS;
		else if (funcName == "sin")
			return SysCall::SIN;
		else if (funcName == "tan")
			return SysCall::TAN;
		else if (funcName == "rand_int")
			return SysCall::RAND_INT;
		else if (funcName == "rand_float")
			return SysCall::RAND_FLOAT;
		else if (funcName == "rand_seed")
			return SysCall::RAND_SEED;

		return SysCall::UNKNOWN;
	}

	/*static*/ uint32_t SysCallInstruction::ArgumentCount(SysCall sysCall)
	{
		switch (sysCall)
		{
			case SysCall::EXIT:
			case SysCall::GC:
			case SysCall::GC_COUNT:
			case SysCall::INPUT:
				return 0;
			case SysCall::OUTPUT:
			case SysCall::MODULE:
			case SysCall::RUN_SCRIPT:
			case SysCall::SLEEP:
			case SysCall::AS_ITERATOR:
			case SysCall::AS_STRING:
			case SysCall::AS_NUMBER:
			case SysCall::COS:
			case SysCall::SIN:
			case SysCall::TAN:
			case SysCall::RAND_SEED:
				return 1;
			case SysCall::RAND_INT:
			case SysCall::RAND_FLOAT:
				return 2;
		}

		return -1;
	}

	/*virtual*/ uint32_t SysCallInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
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
				GC::GarbageCollector::Get()->Collect();
				if (!executor->PushValueOntoEvaluationStackTop(new NullValue(), error))
					return Executor::Result::RUNTIME_ERROR;
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
						value = new NullValue();
					}
				}

				if (!executor->PushValueOntoEvaluationStackTop(value, error))
					return Executor::Result::RUNTIME_ERROR;

				break;
			}
			case SysCall::OUTPUT:
			{
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				std::string str = valueRef.Get()->ToString();
				virtualMachine->GetIODevice()->OutputString(str);
				if (!executor->PushValueOntoEvaluationStackTop(new NumberValue(str.length()), error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			case SysCall::MODULE:
			{
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				std::string moduleRelativePath = valueRef.Get()->ToString();
				std::string moduleAbsolutePath = pathResolver.ResolvePath(moduleRelativePath, PathResolver::SEARCH_BASE | PathResolver::SEARCH_CWD, error);
				if (moduleAbsolutePath.size() == 0)
					return Executor::Result::RUNTIME_ERROR;
				MapValue* functionMapValue = virtualMachine->LoadModuleFunctionMap(moduleAbsolutePath, error);
				if (!functionMapValue)
				{
					error.Add(std::format("Module ({}) did not generate function map value.", moduleAbsolutePath.c_str()));
					return Executor::Result::RUNTIME_ERROR;
				}
				if (!executor->PushValueOntoEvaluationStackTop(functionMapValue, error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			case SysCall::RUN_SCRIPT:
			{
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				std::string scriptRelativePath = valueRef.Get()->ToString();
				std::string scriptAbsolutePath = pathResolver.ResolvePath(scriptRelativePath, PathResolver::SEARCH_CWD, error);
				if (scriptAbsolutePath.size() == 0)
					return Executor::Result::RUNTIME_ERROR;
				if (!virtualMachine->ExecuteSourceCodeFile(scriptAbsolutePath.c_str(), error, executor->GetCurrentScope()))
					return Executor::Result::RUNTIME_ERROR;
				if (!executor->PushValueOntoEvaluationStackTop(new NullValue(), error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			case SysCall::SLEEP:
			{
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				double sleepSeconds = valueRef.Get()->AsNumber();
				if (sleepSeconds > 0.0)
					::Sleep(DWORD(sleepSeconds * 1000.0f));
				if (!executor->PushValueOntoEvaluationStackTop(new NullValue(), error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			case SysCall::GC_COUNT:
			{
				uint32_t count = GC::GarbageCollector::Get()->NumTrackedObjects();
				if (!executor->PushValueOntoEvaluationStackTop(new NumberValue(count), error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			case SysCall::COS:
			case SysCall::SIN:
			case SysCall::TAN:
			{
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				double input = valueRef.Get()->AsNumber();
				double output = 0.0;
				switch (sysCallCode)
				{
					case SysCall::COS: output = ::cos(input); break;
					case SysCall::SIN: output = ::sin(input); break;
					case SysCall::TAN: output = ::tan(input); break;
				}
				if (!executor->PushValueOntoEvaluationStackTop(new NumberValue(output), error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			case SysCall::RAND_INT:
			case SysCall::RAND_FLOAT:
			{
				GC::Reference<Value, true> minValueRef, maxValueRef;
				if (!executor->PopValueFromEvaluationStackTop(maxValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				if (!executor->PopValueFromEvaluationStackTop(minValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				double minValue = minValueRef.Get()->AsNumber();
				double maxValue = maxValueRef.Get()->AsNumber();
				double alpha = double(rand()) / double(RAND_MAX);
				double randValue = minValue + alpha * (maxValue - minValue);
				if (randValue < minValue)
					randValue = minValue;
				if (randValue > maxValue)
					randValue = maxValue;
				if (sysCallCode == SysCall::RAND_INT)
					randValue = ::round(randValue);
				NumberValue* numberValue = new NumberValue(randValue);
				if (!executor->PushValueOntoEvaluationStackTop(numberValue, error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			case SysCall::RAND_SEED:
			{
				Value* seedValue = executor->StackTop(error);
				if (!seedValue)
					return Executor::Result::RUNTIME_ERROR;
				::srand((int)seedValue->AsNumber());
				break;
			}
			case SysCall::AS_ITERATOR:
			{
				Value* value = executor->StackTop(error);
				if (!value)
					return Executor::Result::RUNTIME_ERROR;
				ContainerValue* containerValue = dynamic_cast<ContainerValue*>(value);
				if (containerValue)
				{
					CppFunctionValue* iteratorValue = containerValue->MakeIterator();
					if (iteratorValue)
					{
						GC::Reference<Value, true> poppedValueRef;
						if (!executor->PopValueFromEvaluationStackTop(poppedValueRef, error))
							return Executor::Result::RUNTIME_ERROR;
						if (!executor->PushValueOntoEvaluationStackTop(iteratorValue, error))
							return Executor::Result::RUNTIME_ERROR;
					}
				}

				value = executor->StackTop(error);
				if (!value)
					return Executor::Result::RUNTIME_ERROR;
				if (!dynamic_cast<CppFunctionValue*>(value) && !dynamic_cast<AddressValue*>(value))
				{
					error.Add("Eval stack top value is not callable, so it's not an iterator.");
					return Executor::Result::RUNTIME_ERROR;
				}

				break;
			}
			case SysCall::AS_STRING:
			{
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				StringValue* stringValue = new StringValue(valueRef.Get()->ToString());
				if (!executor->PushValueOntoEvaluationStackTop(stringValue, error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			case SysCall::AS_NUMBER:
			{
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				NumberValue* numberValue = new NumberValue(valueRef.Get()->AsNumber());
				if (!executor->PushValueOntoEvaluationStackTop(numberValue, error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			default:
			{
				error.Add(std::format("Encountered unknown system call: {}", sysCallCode));
				return Executor::Result::RUNTIME_ERROR;
			}
		}

		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool SysCallInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* sysCallEntry = this->assemblyData->configMap.LookupPtr("sysCall");
			if (!sysCallEntry)
			{
				error.Add(std::string(this->assemblyData->fileLocation) + "System call instruction can't be assembled without knowing what system call to call.");
				return false;
			}

			uint8_t* programBuffer = executable->byteCodeBuffer;
			programBuffer[programBufferLocation + 1] = sysCallEntry->code;
		}

		programBufferLocation += 2;
		return true;
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
			detail += std::format("{}", sysCallEntry->code);
		return detail;
	}
#endif
}