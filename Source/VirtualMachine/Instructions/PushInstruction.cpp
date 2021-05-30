#include "PushInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "UndefinedValue.h"
#include "StringValue.h"
#include "NumberValue.h"
#include "ListValue.h"

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
		DataType dataType = DataType(programBuffer[programBufferLocation + 1]);

		switch (dataType)
		{
			case DataType::UNDEFINED:
			{
				executor->GetCurrentScope()->PushValueOntoEvaluationStackTop(new UndefinedValue());
				programBufferLocation += 2;
				break;
			}
			case DataType::STRING:
			{
				uint64_t i = programBufferLocation + 2;
				std::string str;
				while (programBuffer[i] != '\0')
					str += programBuffer[i++];
				executor->GetCurrentScope()->PushValueOntoEvaluationStackTop(new StringValue(str));
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
		}

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void PushInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		const AssemblyData::Entry* dataEntry = this->assemblyData->configMap.LookupPtr("data");

		if (!typeEntry || !dataEntry)
		{
			// TODO: Throw an exception here.
		}

		if (assemblyPass == AssemblyPass::RENDER)
		{
			if (typeEntry->string == "undefined")
				programBuffer[programBufferLocation + 1] = uint8_t(DataType::UNDEFINED);
			else if (typeEntry->string == "string")
			{
				programBuffer[programBufferLocation + 1] = uint8_t(DataType::STRING);
				::memcpy_s(&programBuffer[programBufferLocation + 2], dataEntry->string.length(), dataEntry->string.c_str(), dataEntry->string.length());
				programBuffer[programBufferLocation + 2 + dataEntry->string.length()] = '\0';
			}
			else if (typeEntry->string == "number")
			{
				programBuffer[programBufferLocation + 1] = uint8_t(DataType::NUMBER);
				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(double), &dataEntry->number, sizeof(double));
			}
			else if (typeEntry->string == "emptyList")
				programBuffer[programBufferLocation + 1] = uint8_t(DataType::EMPTY_LIST);
			else
			{
				// TODO: Throw exception.
			}
		}

		programBufferLocation += 2L;

		if (typeEntry->string == "undefined")
			programBufferLocation += 0L;
		else if (typeEntry->string == "string")
			programBufferLocation += uint64_t(dataEntry->string.length()) + 1L;
		else if (typeEntry->string == "number")
			programBufferLocation += sizeof(double);
		else if (typeEntry->string == "emptyList")
			programBufferLocation += 0L;
	}
}