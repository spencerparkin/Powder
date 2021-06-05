#include "JumpInstruction.h"
#include "Scope.h"
#include "AddressValue.h"
#include "Assembler.h"
#include "Exceptions.hpp"
#include "Executor.h"

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

	/*virtual*/ uint32_t JumpInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
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
				Value* value = executor->PopValueFromEvaluationStackTop();
				AddressValue* addressValue = dynamic_cast<AddressValue*>(value);
				if (!addressValue)
					throw new RunTimeException("Cannot jump to location indicated by anything other than an address value.");

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
			throw new CompileTimeException("Can't assemble jump instruction if not given the jump-type.");

		if (assemblyPass == AssemblyPass::RENDER)
		{
			programBuffer[programBufferLocation + 1] = typeEntry->code;

			if (typeEntry->code == Type::JUMP_TO_EMBEDDED_ADDRESS)
			{
				const AssemblyData::Entry* jumpEntry = this->assemblyData->configMap.LookupPtr("jump");
				if (!jumpEntry)
					throw new CompileTimeException("Can't assemble jump instruction to hard-coded program location if not given that location.");

				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(uint64_t), &jumpEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
			}
		}

		programBufferLocation += 2;
		if (typeEntry->code == Type::JUMP_TO_EMBEDDED_ADDRESS)
			programBufferLocation += sizeof(uint64_t);
	}
}