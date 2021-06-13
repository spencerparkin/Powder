#include "MapInstruction.h"
#include "Assembler.h"
#include "Executor.h"
#include "MapValue.h"
#include "Exceptions.hpp"

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

	/*virtual*/ uint32_t MapInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		Value* result = nullptr;

		uint8_t action = programBuffer[programBufferLocation + 1];
		switch (action)
		{
			case Action::REMOVE:
			{
				Value* fieldValue = executor->PopValueFromEvaluationStackTop();
				MapValue* mapValue = dynamic_cast<MapValue*>(executor->PopValueFromEvaluationStackTop());
				if (!mapValue)
					throw new RunTimeException("Map instruction can only remove elements from a map value.");

				mapValue->DelField(fieldValue);
				
				// It's most convenient, I think, to leave the map value on the stack.
				result = mapValue;
				break;
			}
			case Action::INSERT:
			{
				Value* dataValue = executor->PopValueFromEvaluationStackTop();
				Value* fieldValue = executor->PopValueFromEvaluationStackTop();
				MapValue* mapValue = dynamic_cast<MapValue*>(executor->PopValueFromEvaluationStackTop());
				if (!mapValue)
					throw new RunTimeException("Map instruction can only insert elements into a map value.");

				mapValue->SetField(fieldValue, dataValue);

				// It's most convenient, I think, to leave the map value on the stack.
				result = mapValue;
				break;
			}
			case Action::MAKE_KEY_LIST:
			{
				MapValue* mapValue = dynamic_cast<MapValue*>(executor->PopValueFromEvaluationStackTop());
				if (!mapValue)
					throw new RunTimeException("Map instruction can only generate key lists for map values.");

				result = mapValue->GenerateKeyListValue();
				break;
			}
			default:
			{
				break;
			}
		}

		if (!result)
			throw new RunTimeException("Map instruction had no result.");

		executor->PushValueOntoEvaluationStackTop(result);

		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void MapInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* actionEntry = this->assemblyData->configMap.LookupPtr("action");
		if (!actionEntry)
			throw new CompileTimeException("Assembly of map instruction failed because no action was given.");

		if (assemblyPass == AssemblyPass::RENDER)
		{
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
					throw new CompileTimeException(FormatString("Did not recognize map instruction code: 0x%04x", actionEntry->code));
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