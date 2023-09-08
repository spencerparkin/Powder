#include "MapInstruction.h"
#include "Assembler.h"
#include "Executor.h"
#include "MapValue.h"
#include "ListValue.h"
#include "Executable.h"
#include "VirtualMachine.h"

namespace Powder
{
	MapInstruction::MapInstruction()
	{
	}

	/*virtual*/ MapInstruction::~MapInstruction()
	{
	}

	/*virtual*/ uint8_t MapInstruction::OpCode() const
	{
		return 0x0D;
	}

	/*virtual*/ uint32_t MapInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		Value* result = nullptr;

		uint8_t* programBuffer = executable->byteCodeBuffer;
		uint8_t action = programBuffer[programBufferLocation + 1];
		switch (action)
		{
			case Action::REMOVE:
			{
				// Notice we pop the field value, but leave the map value.
				GC::Reference<Value, true> fieldValueRef;
				if (!executor->PopValueFromEvaluationStackTop(fieldValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				Value* value = executor->StackTop(error);
				if (!value)
					return Executor::Result::RUNTIME_ERROR;
				MapValue* mapValue = dynamic_cast<MapValue*>(value);
				if (!mapValue)
				{
					error.Add("Map instruction can only remove elements from a map value.");
					return Executor::Result::RUNTIME_ERROR;
				}
				GC::Reference<Value, true> dataValueRef;
				if (!mapValue->DelField(fieldValueRef.Get(), dataValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				if (virtualMachine->GetDebuggerTrap())
					virtualMachine->GetDebuggerTrap()->ValueChanged(mapValue);
				break;
			}
			case Action::INSERT:
			{
				// Notice we pop the field and data values, but leave the map value.
				GC::Reference<Value, true> dataValueRef, fieldValueRef;
				if (!executor->PopValueFromEvaluationStackTop(dataValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				if (!executor->PopValueFromEvaluationStackTop(fieldValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				Value* value = executor->StackTop(error);
				if (!value)
					return Executor::Result::RUNTIME_ERROR;
				MapValue* mapValue = dynamic_cast<MapValue*>(value);
				if (!mapValue)
				{
					error.Add("Map instruction can only insert elements into a map value.");
					return Executor::Result::RUNTIME_ERROR;
				}
				if (!mapValue->SetField(fieldValueRef.Get(), dataValueRef.Get(), error))
					return Executor::Result::RUNTIME_ERROR;
				if (virtualMachine->GetDebuggerTrap())
					virtualMachine->GetDebuggerTrap()->ValueChanged(mapValue);
				break;
			}
			case Action::MAKE_KEY_LIST:
			{
				// In this case, the map value is replaced with the list value.
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				MapValue* mapValue = dynamic_cast<MapValue*>(valueRef.Get());
				if (!mapValue)
				{
					error.Add("Map instruction can only generate key lists for map values.");
					return Executor::Result::RUNTIME_ERROR;
				}
				ListValue* listValue = mapValue->GenerateKeyListValue();
				if (!executor->PushValueOntoEvaluationStackTop(listValue, error))
					return Executor::Result::RUNTIME_ERROR;
				break;
			}
			default:
			{
				error.Add(std::format("Map instruction encountered unknown action {}.", action));
				return Executor::Result::RUNTIME_ERROR;
			}
		}

		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool MapInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		const AssemblyData::Entry* actionEntry = this->assemblyData->configMap.LookupPtr("action");
		if (!actionEntry)
		{
			error.Add(std::string(this->assemblyData->fileLocation) + "Assembly of map instruction failed because no action was given.");
			return false;
		}

		if (assemblyPass == AssemblyPass::RENDER)
		{
			uint8_t* programBuffer = executable->byteCodeBuffer;
			switch (actionEntry->code)
			{
				case Action::INSERT:
				case Action::REMOVE:
				case Action::MAKE_KEY_LIST:
				{
					programBuffer[programBufferLocation + 1] = actionEntry->code;
					break;
				}
				default:
				{
					error.Add(std::string(this->assemblyData->fileLocation) + std::format("Did not recognize map instruction code: {}", actionEntry->code));
					return false;
				}
			}
		}

		programBufferLocation += 2;
		return true;
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string MapInstruction::Print(void) const
	{
		std::string detail;
		detail += "map: ";
		const AssemblyData::Entry* actionEntry = this->assemblyData->configMap.LookupPtr("action");
		if (!actionEntry)
			detail += "?";
		else
			detail += std::format("{}", actionEntry->code);
		return detail;
	}
#endif
}