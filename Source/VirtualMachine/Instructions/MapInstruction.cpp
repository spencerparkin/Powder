#include "MapInstruction.h"
#include "Assembler.h"
#include "Executor.h"
#include "MapValue.h"
#include "ListValue.h"
#include "Exceptions.hpp"
#include "Executable.h"

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

	/*virtual*/ uint32_t MapInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		Value* result = nullptr;

		uint8_t* programBuffer = executable->byteCodeBuffer;
		uint8_t action = programBuffer[programBufferLocation + 1];
		switch (action)
		{
			case Action::REMOVE:
			{
				// Notice we pop the field value, but leave the map value.
				Value* fieldValue = executor->PopValueFromEvaluationStackTop();
				MapValue* mapValue = dynamic_cast<MapValue*>(executor->StackTop());
				if (!mapValue)
					throw new RunTimeException("Map instruction can only remove elements from a map value.");
				mapValue->DelField(fieldValue);
				break;
			}
			case Action::INSERT:
			{
				// Notice we pop the field and data values, but leave the map value.
				Value* dataValue = executor->PopValueFromEvaluationStackTop();
				Value* fieldValue = executor->PopValueFromEvaluationStackTop();
				MapValue* mapValue = dynamic_cast<MapValue*>(executor->StackTop());
				if (!mapValue)
					throw new RunTimeException("Map instruction can only insert elements into a map value.");
				mapValue->SetField(fieldValue, dataValue);
				break;
			}
			case Action::MAKE_KEY_LIST:
			{
				// In this case, the map value is replaced with the list value.
				MapValue* mapValue = dynamic_cast<MapValue*>(executor->PopValueFromEvaluationStackTop());
				if (!mapValue)
					throw new RunTimeException("Map instruction can only generate key lists for map values.");
				ListValue* listValue = mapValue->GenerateKeyListValue();
				executor->PushValueOntoEvaluationStackTop(listValue);
				break;
			}
			default:
			{
				throw new RunTimeException(FormatString("Map instruction encountered unknown action %04d.", action));
			}
		}

		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void MapInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* actionEntry = this->assemblyData->configMap.LookupPtr("action");
		if (!actionEntry)
			throw new CompileTimeException("Assembly of map instruction failed because no action was given.", &this->assemblyData->fileLocation);

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
					throw new CompileTimeException(FormatString("Did not recognize map instruction code: 0x%04x", actionEntry->code), &this->assemblyData->fileLocation);
				}
			}
		}

		programBufferLocation += 2;
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
			detail += FormatString("%04d", actionEntry->code);
		return detail;
	}
#endif
}