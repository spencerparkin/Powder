#include "JumpInstruction.h"
#include "Scope.h"
#include "AddressValue.h"
#include "Assembler.h"

namespace Powder
{
	JumpInstruction::JumpInstruction()
	{
	}

	/*virtual*/ JumpInstruction::~JumpInstruction()
	{
	}

	/*virtual*/ uint8_t JumpInstruction::OpCode() const
	{
		return 0x03;
	}

	/*virtual*/ Executor::Result JumpInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		Type type = Type(programBuffer[programBufferLocation + 1]);

		switch (type)
		{
			case Type::JUMP_TO_EMBEDDED_ADDRESS:
			{
				::memcpy(&programBufferLocation, &programBuffer[programBufferLocation + 2], sizeof(uint64_t));
				break;
			}
			case Type::JUMP_TO_LOADED_ADDRESS:
			{
				Value* value = nullptr;
				executor->GetCurrentScope()->PopValueFromEvaluationStackTop(value);

				if (!value)
				{
					// TODO: Throw an exception.
				}

				AddressValue* addressValue = dynamic_cast<AddressValue*>(value);
				if (!addressValue)
				{
					// TODO: Throw an exception.
				}

				programBufferLocation = *addressValue;
				break;
			}
		}
		
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void JumpInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		if (!typeEntry)
		{
			// TODO: Throw an exception.
		}

		if (assemblyPass == AssemblyPass::RENDER)
		{
			if (typeEntry->string == "jump_to_loaded_address")
				programBuffer[programBufferLocation + 1] = uint8_t(Type::JUMP_TO_LOADED_ADDRESS);
			else if (typeEntry->string == "jump_to_embedded_address")
			{
				programBuffer[programBufferLocation + 1] = uint8_t(Type::JUMP_TO_EMBEDDED_ADDRESS);

				const AssemblyData::Entry* jumpEntry = this->assemblyData->configMap.LookupPtr("jump");
				if (!jumpEntry)
				{
					// TODO: Throw an exception.
				}

				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(uint64_t), &jumpEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
			}
		}

		programBufferLocation += 2;

		if (typeEntry->string == "jump_to_embedded_address")
			programBufferLocation += sizeof(uint64_t);
	}
}