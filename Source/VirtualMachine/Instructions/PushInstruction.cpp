#include "PushInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "UndefinedValue.h"
#include "StringValue.h"
#include "NumberValue.h"
#include "ListValue.h"
#include "VariableValue.h"
#include "Exceptions.hpp"

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

	/*virtual*/ Executor::Result PushInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		uint8_t pushType = programBuffer[programBufferLocation + 1];
		switch (pushType)
		{
			case DataType::UNDEFINED:
			{
				executor->GetCurrentScope()->PushValueOntoEvaluationStackTop(new UndefinedValue());
				programBufferLocation += 2;
				break;
			}
			case DataType::STRING:
			case DataType::VARIABLE:
			{
				uint64_t i = programBufferLocation + 2;
				std::string str;
				while (programBuffer[i] != '\0')
					str += programBuffer[i++];
				Value* value = (pushType == DataType::STRING) ? (Value*)new StringValue(str) : (Value*)new VariableValue(str);
				executor->GetCurrentScope()->PushValueOntoEvaluationStackTop(value);
				programBufferLocation += 2 + str.size() + 1;
				break;
			}
			case DataType::NUMBER:
			{
				double number = 0.0f;
				::memcpy_s(&number, sizeof(double), &programBuffer[programBufferLocation + 2], sizeof(double));
				executor->GetCurrentScope()->PushValueOntoEvaluationStackTop(new NumberValue(number));
				programBufferLocation += 2 + sizeof(double);
				break;
			}
			case DataType::EMPTY_LIST:
			{
				executor->GetCurrentScope()->PushValueOntoEvaluationStackTop(new ListValue());
				programBufferLocation += 2;
				break;
			}
			default:
			{
				throw new RunTimeException("Encountered uknown push type 0x%04x");
			}
		}

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void PushInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		const AssemblyData::Entry* dataEntry = this->assemblyData->configMap.LookupPtr("data");

		if (!typeEntry)
			throw new CompileTimeException("Can't assemble push instruction if not given type information.");

		if (!dataEntry)
			throw new CompileTimeException("Can't assemble push instruction if not given data information.");

		if (assemblyPass == AssemblyPass::RENDER)
		{
			programBuffer[programBufferLocation + 1] = typeEntry->code;

			if (typeEntry->code == DataType::STRING || typeEntry->code == DataType::VARIABLE)
			{
				::memcpy_s(&programBuffer[programBufferLocation + 2], dataEntry->string.length(), dataEntry->string.c_str(), dataEntry->string.length());
				programBuffer[programBufferLocation + 2 + dataEntry->string.length()] = '\0';
			}
			else if (typeEntry->code == DataType::NUMBER)
				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(double), &dataEntry->number, sizeof(double));
			else if (typeEntry->code == DataType::ADDRESS)
				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(uint64_t), &dataEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
		}

		programBufferLocation += 2L;
		if (typeEntry->code == DataType::UNDEFINED || typeEntry->code == DataType::EMPTY_LIST)
			programBufferLocation += 0L;
		else if (typeEntry->code == DataType::STRING || typeEntry->code == DataType::VARIABLE)
			programBufferLocation += uint64_t(dataEntry->string.length()) + 1L;
		else if (typeEntry->code == DataType::NUMBER)
			programBufferLocation += sizeof(double);
		else if (typeEntry->code == DataType::ADDRESS)
			programBufferLocation += sizeof(uint64_t);
	}
}