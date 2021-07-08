#include "PushInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "UndefinedValue.h"
#include "StringValue.h"
#include "NumberValue.h"
#include "ListValue.h"
#include "MapValue.h"
#include "AddressValue.h"
#include "ClosureValue.h"
#include "Exceptions.hpp"
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

	/*virtual*/ uint32_t PushInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		uint8_t* programBuffer = executable->byteCodeBuffer;
		uint8_t pushType = programBuffer[programBufferLocation + 1];
		switch (pushType)
		{
			case DataType::UNDEFINED:
			{
				executor->PushValueOntoEvaluationStackTop(new UndefinedValue());
				programBufferLocation += 2;
				break;
			}
			case DataType::STRING:
			{
				std::string str = this->ExtractEmbeddedString(programBuffer, programBufferLocation + 2);
				Value* value = new StringValue(str);
				executor->PushValueOntoEvaluationStackTop(value);
				programBufferLocation += 2 + str.length() + 1;
				break;
			}
			case DataType::NUMBER:
			{
				double number = 0.0f;
				::memcpy_s(&number, sizeof(double), &programBuffer[programBufferLocation + 2], sizeof(double));
				executor->PushValueOntoEvaluationStackTop(new NumberValue(number));
				programBufferLocation += 2 + sizeof(double);
				break;
			}
			case DataType::EMPTY_LIST:
			{
				executor->PushValueOntoEvaluationStackTop(new ListValue());
				programBufferLocation += 2;
				break;
			}
			case DataType::EMPTY_MAP:
			{
				executor->PushValueOntoEvaluationStackTop(new MapValue());
				programBufferLocation += 2;
				break;
			}
			case DataType::ADDRESS:
			case DataType::CLOSURE:
			{
				uint64_t programBufferAddress = 0L;
				::memcpy_s(&programBufferAddress, sizeof(uint64_t), &programBuffer[programBufferLocation + 2], sizeof(uint64_t));
				AddressValue* addressValue = (pushType == DataType::ADDRESS) ? new AddressValue(executable, programBufferAddress) : new ClosureValue(executable, programBufferAddress);
				executor->PushValueOntoEvaluationStackTop(addressValue);
				programBufferLocation += 2 + sizeof(uint64_t);
				break;
			}
			default:
			{
				throw new RunTimeException(FormatString("Encountered uknown push type: 0x%04x", pushType));
			}
		}

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void PushInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		const AssemblyData::Entry* dataEntry = this->assemblyData->configMap.LookupPtr("data");

		if (!typeEntry)
			throw new CompileTimeException("Can't assemble push instruction if not given type information.", &this->assemblyData->fileLocation);

		if (!dataEntry && typeEntry->code != DataType::UNDEFINED && typeEntry->code != DataType::EMPTY_LIST && typeEntry->code != DataType::EMPTY_MAP)
			throw new CompileTimeException("Some push instructions can't be assembled without being given more information about the push content.", &this->assemblyData->fileLocation);

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
			else if (typeEntry->code == DataType::ADDRESS || typeEntry->code == DataType::CLOSURE)
				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(uint64_t), &dataEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
		}

		programBufferLocation += 2L;
		if (typeEntry->code == DataType::UNDEFINED || typeEntry->code == DataType::EMPTY_LIST || typeEntry->code == DataType::EMPTY_MAP)
			programBufferLocation += 0L;
		else if (typeEntry->code == DataType::STRING)
			programBufferLocation += uint64_t(dataEntry->string.length()) + 1L;
		else if (typeEntry->code == DataType::NUMBER)
			programBufferLocation += sizeof(double);
		else if (typeEntry->code == DataType::ADDRESS || typeEntry->code == DataType::CLOSURE)
			programBufferLocation += sizeof(uint64_t);
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string PushInstruction::Print(void) const
	{
		std::string detail;
		detail += "push: ";
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		detail += typeEntry ? FormatString("type: %04d", typeEntry->code) : "type: ????";
		const AssemblyData::Entry* dataEntry = this->assemblyData->configMap.LookupPtr("data");
		if (dataEntry)
		{
			detail += "; data: ";
			if (typeEntry->code == DataType::STRING)
				detail += dataEntry->string;
			else if (typeEntry->code == DataType::NUMBER)
				detail += FormatString("%f", dataEntry->number);
			else if (typeEntry->code == ADDRESS || typeEntry->code == CLOSURE)
				detail += FormatString("%04d", dataEntry->instruction->assemblyData->programBufferLocation);
			else if (typeEntry->code == UNDEFINED)
				detail += "undef";
			else if (typeEntry->code == DataType::EMPTY_LIST)
				detail += "[]";
			else if (typeEntry->code == DataType::EMPTY_MAP)
				detail += "{}";
			else
				detail += "?";
		}
		return detail;
	}
#endif
}