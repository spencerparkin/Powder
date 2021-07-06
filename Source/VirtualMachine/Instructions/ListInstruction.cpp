#include "ListInstruction.h"
#include "Assembler.h"
#include "Executor.h"
#include "ListValue.h"
#include "Exceptions.hpp"
#include "Executable.h"

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

	/*virtual*/ uint32_t ListInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		Value* result = nullptr;

		const uint8_t* programBuffer = executable->byteCodeBuffer;
		uint8_t action = programBuffer[programBufferLocation + 1];
		switch (action)
		{
			case Action::POP_LEFT:
			case Action::POP_RIGHT:
			{
				// Notice we push the popped value, but leave the list value.
				ListValue* listValue = dynamic_cast<ListValue*>(executor->StackTop());
				if (!listValue)
					throw new RunTimeException("List instruction can only pop elements from a list value.");
				Value* elementValue = (action == Action::POP_LEFT) ? listValue->PopLeft() : listValue->PopRight();
				executor->PushValueOntoEvaluationStackTop(elementValue);
				break;
			}
			case Action::PUSH_LEFT:
			case Action::PUSH_RIGHT:
			{
				// Notice that we pop the pushed value, but leave the list value.
				Value* elementValue = executor->PopValueFromEvaluationStackTop();
				ListValue* listValue = dynamic_cast<ListValue*>(executor->StackTop());
				if (!listValue)
					throw new RunTimeException("List instruction can only push elements to a list value.");
				if (action == Action::PUSH_LEFT)
					listValue->PushLeft(elementValue);
				else
					listValue->PushRight(elementValue);
				break;
			}
			default:
			{
				throw new RunTimeException(FormatString("List instruction encountered unknown action %04d.", action));
			}
		}

		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void ListInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		const AssemblyData::Entry* actionEntry = this->assemblyData->configMap.LookupPtr("action");
		if (!actionEntry)
			throw new CompileTimeException("Assembly of list instruction failed because no action was given.", &this->assemblyData->fileLocation);

		if (assemblyPass == AssemblyPass::RENDER)
		{
			uint8_t* programBuffer = executable->byteCodeBuffer;
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
					throw new CompileTimeException(FormatString("Did not recognize list instruction code: 0x%04x", actionEntry->code), &this->assemblyData->fileLocation);
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