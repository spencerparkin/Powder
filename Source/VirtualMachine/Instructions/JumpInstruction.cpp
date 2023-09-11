#include "JumpInstruction.h"
#include "Scope.h"
#include "AddressValue.h"
#include "ClosureValue.h"
#include "Assembler.h"
#include "Executor.h"
#include "Executable.h"
#include "CppFunctionValue.h"
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

	/*virtual*/ uint32_t JumpInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		const uint8_t* programBuffer = executable->byteCodeBuffer;
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
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;

				AddressValue* addressValue = dynamic_cast<AddressValue*>(valueRef.Get());
				if (addressValue)
				{
					programBufferLocation = addressValue->programBufferLocation;
					executable = addressValue->executableRef.Get();

					ClosureValue* closureValue = dynamic_cast<ClosureValue*>(addressValue);
					if (closureValue)
					{
						closureValue->scopeRef.Get()->Absorb(executor->GetCurrentScope());		// Grab the return address(!) among, perhaps, other things.
						executor->ReplaceCurrentScope(closureValue->scopeRef.Get());
					}

					break;
				}
				
				CppFunctionValue* cppFunctionValue = dynamic_cast<CppFunctionValue*>(valueRef.Get());
				if (cppFunctionValue)
				{
					GC::Reference<Value, true> argValueRef;
					if (!executor->PopValueFromEvaluationStackTop(argValueRef, error))
						return Executor::Result::RUNTIME_ERROR;

					ListValue* argListValue = dynamic_cast<ListValue*>(argValueRef.Get());
					if (!argListValue)
					{
						error.Add("Did not get argument list value from evaluation stack top for module function call.");
						return Executor::Result::RUNTIME_ERROR;
					}

					std::string errorMsg;
					GC::Reference<Value, true> resultValueRef;
					if (!cppFunctionValue->Call(argListValue, resultValueRef, error))
						return Executor::Result::RUNTIME_ERROR;

					if (!resultValueRef.Get())
						resultValueRef.Set(new UndefinedValue());

					if (!executor->PushValueOntoEvaluationStackTop(resultValueRef.Get(), error))
						return Executor::Result::RUNTIME_ERROR;

					programBufferLocation += 2;
					break;
				}
				
				error.Add("Cannot jump to location indicated by anything other than an address value.");
				return Executor::Result::RUNTIME_ERROR;
			}
		}
		
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool JumpInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		const AssemblyData::Entry* typeEntry = this->assemblyData->configMap.LookupPtr("type");
		if (!typeEntry)
		{
			error.Add(std::string(this->assemblyData->fileLocation) + "Can't assemble jump instruction if not given the jump-type.");
			return false;
		}

		if (assemblyPass == AssemblyPass::RENDER)
		{
			uint8_t* programBuffer = executable->byteCodeBuffer;
			programBuffer[programBufferLocation + 1] = typeEntry->code;

			if (typeEntry->code == Type::JUMP_TO_EMBEDDED_ADDRESS)
			{
				const AssemblyData::Entry* jumpEntry = this->assemblyData->configMap.LookupPtr("jump");
				if (!jumpEntry)
				{
					error.Add(std::string(this->assemblyData->fileLocation) + "Can't assemble jump instruction to hard-coded program location if not given that location.");
					return false;
				}

				if (!jumpEntry->instruction)
				{
					error.Add(std::string(this->assemblyData->fileLocation) + "Jump instruction was not resolved.");
					return false;
				}

				::memcpy_s(&programBuffer[programBufferLocation + 2], sizeof(uint64_t), &jumpEntry->instruction->assemblyData->programBufferLocation, sizeof(uint64_t));
			}
		}

		programBufferLocation += 2;
		if (typeEntry->code == Type::JUMP_TO_EMBEDDED_ADDRESS)
			programBufferLocation += sizeof(uint64_t);

		return true;
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
				if (jumpEntry && jumpEntry->instruction)
					detail += std::format("{:#08x}", jumpEntry->instruction->assemblyData->programBufferLocation);
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