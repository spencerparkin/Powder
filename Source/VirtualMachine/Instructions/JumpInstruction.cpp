#include "JumpInstruction.h"
#include "Scope.h"
#include "AddressValue.h"
#include "Assembler.h"
#include "Exceptions.hpp"
#include "Executor.h"
#include "StringValue.h"
#include "ExtensionModule.h"
#include "ListValue.h"
#include "UndefinedValue.h"
#include "VirtualMachine.h"

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
				if (addressValue)
				{
					programBufferLocation = *addressValue;
					break;
				}
				
				// An odd consequence of this is that "blah"() is an alternative syntax to @blah().
				StringValue* stringValue = dynamic_cast<StringValue*>(value);
				if (stringValue)
				{
					std::string funcName = stringValue->ToString();
					ExtensionModule::Function* moduleFunction = virtualMachine->LookupModuleFunction(funcName);
					if (!moduleFunction)
						throw new RunTimeException(FormatString("Did not find module function \"%s\" among all currently loaded extension modules.", funcName.c_str()));

					ListValue* argListValue = dynamic_cast<ListValue*>(executor->PopValueFromEvaluationStackTop());
					if (!argListValue)
						throw new RunTimeException(FormatString("Did not get argument list value from evaluation stack top for module function call \"%s\".", funcName.c_str()));

					Value* resultValue = moduleFunction->Call(*argListValue);
					if (!resultValue)
						resultValue = new UndefinedValue();

					executor->PushValueOntoEvaluationStackTop(resultValue);
					break;
				}				
				
				throw new RunTimeException("Cannot jump to location indicated by anything other than an address value.");
			}
		}
		
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void JumpInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		if (!typeEntry)
			throw new CompileTimeException("Can't assemble jump instruction if not given the jump-type.", &this->assemblyData->fileLocation);

		if (assemblyPass == AssemblyPass::RENDER)
		{
			programBuffer[programBufferLocation + 1] = typeEntry->code;

			if (typeEntry->code == Type::JUMP_TO_EMBEDDED_ADDRESS)
			{
				const AssemblyData::Entry* jumpEntry = this->assemblyData->configMap.LookupPtr("jump");
				if (!jumpEntry)
					throw new CompileTimeException("Can't assemble jump instruction to hard-coded program location if not given that location.", &this->assemblyData->fileLocation);

				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(uint64_t), &jumpEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
			}
		}

		programBufferLocation += 2;
		if (typeEntry->code == Type::JUMP_TO_EMBEDDED_ADDRESS)
			programBufferLocation += sizeof(uint64_t);
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string JumpInstruction::Print(void) const
	{
		std::string detail;
		detail += "jump: ";
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		if (!typeEntry)
			detail += "?";
		else
		{
			if (typeEntry->code == Type::JUMP_TO_EMBEDDED_ADDRESS)
			{
				const AssemblyData::Entry* jumpEntry = this->assemblyData->configMap.LookupPtr("jump");
				if (jumpEntry)
					detail += FormatString("%04d", jumpEntry->instruction->assemblyData->programBufferLocation);
				else
					detail += "?";
			}
			else if (typeEntry->code == Type::JUMP_TO_LOADED_ADDRESS)
				detail += "loaded address";
			else
				detail += "?";
		}
		return detail;
	}
#endif
}