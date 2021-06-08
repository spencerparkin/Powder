#include "ListInstruction.h"
#include "Assembler.h"
#include "Executor.h"
#include "ListValue.h"
#include "Exceptions.hpp"

namespace Powder
{
	ListInstruction::ListInstruction()
	{
	}

	/*virtual*/ ListInstruction::~ListInstruction()
	{
	}

	/*virtual*/ uint8_t ListInstruction::OpCode() const
	{
		return 0x0C;
	}

	/*virtual*/ uint32_t ListInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		Value* result = nullptr;

		uint8_t action = programBuffer[programBufferLocation + 1];
		switch (action)
		{
			case Action::POP_LEFT:
			case Action::POP_RIGHT:
			{
				ListValue* listValue = dynamic_cast<ListValue*>(executor->PopValueFromEvaluationStackTop());
				if (!listValue)
					throw new RunTimeException("List instruction can only pop elements from a list value.");

				result = (action == Action::POP_LEFT) ? listValue->PopLeft() : listValue->PopRight();
				break;
			}
			case Action::PUSH_LEFT:
			case Action::PUSH_RIGHT:
			{
				Value* elementValue = executor->PopValueFromEvaluationStackTop();
				ListValue* listValue = dynamic_cast<ListValue*>(executor->PopValueFromEvaluationStackTop());
				if (!listValue)
					throw new RunTimeException("List instruction can only push elements to a list value.");

				if (action == Action::PUSH_LEFT)
					listValue->PushLeft(elementValue);
				else
					listValue->PushRight(elementValue);

				// It's most convenient, I think, to leave the list value on the stack.
				result = listValue;
				break;
			}
			default:
			{
				break;
			}
		}

		if (!result)
			throw new RunTimeException("List instruction had no result.");

		executor->PushValueOntoEvaluationStackTop(result);

		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void ListInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* actionEntry = this->assemblyData->configMap.LookupPtr("action");
		if (!actionEntry)
			throw new CompileTimeException("Assembly of list instruction failed because no action was given.");

		if (assemblyPass == AssemblyPass::RENDER)
		{
			switch (actionEntry->code)
			{
				case Action::POP_LEFT:
				case Action::POP_RIGHT:
				case Action::PUSH_LEFT:
				case Action::PUSH_RIGHT:
				{
					programBuffer[programBufferLocation + 1] = actionEntry->code;
					break;
				}
				default:
				{
					throw new CompileTimeException(FormatString("Did not recognize list instruction code: 0x%04x", actionEntry->code));
				}
			}
		}

		programBufferLocation += 2;
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string ListInstruction::Print(void) const
	{
		std::string detail;
		detail += "list: ";
		const AssemblyData::Entry* actionEntry = this->assemblyData->configMap.LookupPtr("action");
		if (!actionEntry)
			detail += "?";
		else
			detail += FormatString("%04d", actionEntry->code);
		return detail;
	}
#endif
}