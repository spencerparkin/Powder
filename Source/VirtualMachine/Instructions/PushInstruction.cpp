#include "PushInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "NullValue.h"
#include "StringValue.h"
#include "NumberValue.h"
#include "BooleanValue.h"
#include "ListValue.h"
#include "MapValue.h"
#include "SetValue.h"
#include "AddressValue.h"
#include "ClosureValue.h"
#include "Executor.h"
#include "Executable.h"

namespace Powder
{
	PushInstruction::PushInstruction()
	{
	}

	/*virtual*/ PushInstruction::~PushInstruction()
	{
	}

	/*virtual*/ uint8_t PushInstruction::OpCode() const
	{
		return 0x07;
	}

	/*virtual*/ uint32_t PushInstruction::Execute(GC::Reference<Executable, false>& executableRef, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		uint8_t* programBuffer = executableRef.Get()->byteCodeBuffer;
		uint8_t pushType = programBuffer[programBufferLocation + 1];
		switch (pushType)
		{
			case DataType::NULL_VALUE:
			{
				if (!executor->PushValueOntoEvaluationStackTop(new NullValue(), error))
					return Executor::Result::RUNTIME_ERROR;
				programBufferLocation += 2;
				break;
			}
			case DataType::STRING:
			{
				std::string str = this->ExtractEmbeddedString(programBuffer, programBufferLocation + 2);
				if (!executor->PushValueOntoEvaluationStackTop(new StringValue(str), error))
					return Executor::Result::RUNTIME_ERROR;
				programBufferLocation += 2 + str.length() + 1;
				break;
			}
			case DataType::NUMBER:
			{
				double number = 0.0f;
				::memcpy_s(&number, sizeof(double), &programBuffer[programBufferLocation + 2], sizeof(double));
				if (!executor->PushValueOntoEvaluationStackTop(new NumberValue(number), error))
					return Executor::Result::RUNTIME_ERROR;
				programBufferLocation += 2 + sizeof(double);
				break;
			}
			case DataType::BOOL:
			{
				bool boolValue = false;
				::memcpy_s(&boolValue, sizeof(bool), &programBuffer[programBufferLocation + 2], sizeof(bool));
				if (!executor->PushValueOntoEvaluationStackTop(new BooleanValue(boolValue), error))
					return Executor::Result::RUNTIME_ERROR;
				programBufferLocation += 2 + sizeof(bool);
				break;
			}
			case DataType::EMPTY_LIST:
			{
				if (!executor->PushValueOntoEvaluationStackTop(new ListValue(), error))
					return Executor::Result::RUNTIME_ERROR;
				programBufferLocation += 2;
				break;
			}
			case DataType::EMPTY_MAP:
			{
				if (!executor->PushValueOntoEvaluationStackTop(new MapValue(), error))
					return Executor::Result::RUNTIME_ERROR;
				programBufferLocation += 2;
				break;
			}
			case DataType::EMPTY_SET:
			{
				if (!executor->PushValueOntoEvaluationStackTop(new SetValue(), error))
					return Executor::Result::RUNTIME_ERROR;
				programBufferLocation += 2;
				break;
			}
			case DataType::ADDRESS:
			case DataType::CLOSURE:
			{
				uint64_t programBufferAddress = 0L;
				::memcpy_s(&programBufferAddress, sizeof(uint64_t), &programBuffer[programBufferLocation + 2], sizeof(uint64_t));
				AddressValue* addressValue = (pushType == DataType::ADDRESS) ? new AddressValue(executableRef.Get(), programBufferAddress) : new ClosureValue(executableRef.Get(), programBufferAddress);
				if (!executor->PushValueOntoEvaluationStackTop(addressValue, error))
					return Executor::Result::RUNTIME_ERROR;
				programBufferLocation += 2 + sizeof(uint64_t);
				break;
			}
			case DataType::EXISTING_VALUE:
			{
				int32_t stackOffset = 0;
				::memcpy_s(&stackOffset, sizeof(int32_t), &programBuffer[programBufferLocation + 2], sizeof(int32_t));
				Value* value = executor->StackValue(stackOffset, &error);
				if (!value)
					return Executor::Result::RUNTIME_ERROR;
				if (!executor->PushValueOntoEvaluationStackTop(value, error))
					return Executor::Result::RUNTIME_ERROR;
				programBufferLocation += 2 + sizeof(int32_t);
				break;
			}
			default:
			{
				error.Add(std::format("Encountered uknown push type: {}", pushType));
				return Executor::Result::RUNTIME_ERROR;
			}
		}

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool PushInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		const AssemblyData::Entry* dataEntry = this->assemblyData->configMap.LookupPtr("data");

		if (!typeEntry)
		{
			error.Add(std::string(this->assemblyData->fileLocation) + "Can't assemble push instruction if not given type information.");
			return false;
		}

		if (!dataEntry &&
			typeEntry->code != DataType::NULL_VALUE &&
			typeEntry->code != DataType::EMPTY_LIST &&
			typeEntry->code != DataType::EMPTY_MAP &&
			typeEntry->code != DataType::EMPTY_SET)
		{
			error.Add(std::string(this->assemblyData->fileLocation) + "Some push instructions can't be assembled without being given more information about the push content.");
			return false;
		}

		if (assemblyPass == AssemblyPass::RENDER)
		{
			uint8_t* programBuffer = executable->byteCodeBuffer;
			programBuffer[programBufferLocation + 1] = typeEntry->code;

			if (typeEntry->code == DataType::STRING)
			{
				::memcpy_s(&programBuffer[programBufferLocation + 2], dataEntry->string.length(), dataEntry->string.c_str(), dataEntry->string.length());
				programBuffer[programBufferLocation + 2 + dataEntry->string.length()] = '\0';
			}
			else if (typeEntry->code == DataType::NUMBER)
				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(double), &dataEntry->number, sizeof(double));
			else if (typeEntry->code == DataType::BOOL)
			{
				bool boolValue = (dataEntry->number == 0.0) ? false : true;
				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(bool), &boolValue, sizeof(bool));
			}
			else if (typeEntry->code == DataType::ADDRESS || typeEntry->code == DataType::CLOSURE)
				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(uint64_t), &dataEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
			else if (typeEntry->code == DataType::EXISTING_VALUE)
				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(int32_t), &dataEntry->offset, sizeof(int32_t));
		}

		programBufferLocation += 2L;
		if (typeEntry->code == DataType::NULL_VALUE || typeEntry->code == DataType::EMPTY_LIST || typeEntry->code == DataType::EMPTY_MAP || typeEntry->code == DataType::EMPTY_SET)
			programBufferLocation += 0L;
		else if (typeEntry->code == DataType::STRING)
			programBufferLocation += uint64_t(dataEntry->string.length()) + 1L;
		else if (typeEntry->code == DataType::NUMBER)
			programBufferLocation += sizeof(double);
		else if (typeEntry->code == DataType::BOOL)
			programBufferLocation += sizeof(bool);
		else if (typeEntry->code == DataType::ADDRESS || typeEntry->code == DataType::CLOSURE)
			programBufferLocation += sizeof(uint64_t);
		else if (typeEntry->code == DataType::EXISTING_VALUE)
			programBufferLocation += sizeof(int32_t);

		return true;
	}

	/*virtual*/ std::string PushInstruction::Print(void) const
	{
		std::string detail;
		detail += "push: ";
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		detail += typeEntry ? std::format("type: {}", typeEntry->code) : "type: ????";
		const AssemblyData::Entry* dataEntry = this->assemblyData->configMap.LookupPtr("data");
		if (dataEntry)
		{
			detail += "; data: ";
			if (typeEntry->code == DataType::STRING)
				detail += dataEntry->string;
			else if (typeEntry->code == DataType::NUMBER)
				detail += std::format("{}", dataEntry->number);
			else if (typeEntry->code == DataType::BOOL)
				detail += (dataEntry->number == 0.0) ? "false" : "true";
			else if (typeEntry->code == ADDRESS || typeEntry->code == CLOSURE)
				detail += std::format("{}", dataEntry->instruction->assemblyData->programBufferLocation);
			else if (typeEntry->code == DataType::NULL_VALUE)
				detail += "null";
			else if (typeEntry->code == DataType::EMPTY_LIST)
				detail += "[]";
			else if (typeEntry->code == DataType::EMPTY_MAP)
				detail += "{}";
			else if (typeEntry->code == DataType::EMPTY_SET)
				detail += "emptyset";
			else if (typeEntry->code == DataType::EXISTING_VALUE)
				detail += std::format("stack({})", dataEntry->offset);
			else
				detail += "?";
		}
		return detail;
	}
}