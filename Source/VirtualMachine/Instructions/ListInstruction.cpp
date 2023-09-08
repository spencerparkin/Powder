#include "ListInstruction.h"
#include "Assembler.h"
#include "Executor.h"
#include "ListValue.h"
#include "Executable.h"
#include "VirtualMachine.h"

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

	/*virtual*/ uint32_t ListInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
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
				Value* value = executor->StackTop(error);
				if (!value)
					return Executor::Result::RUNTIME_ERROR;
				ListValue* listValue = dynamic_cast<ListValue*>(value);
				if (!listValue)
				{
					error.Add("List instruction can only pop elements from a list value.");
					return Executor::Result::RUNTIME_ERROR;
				}
				GC::Reference<Value, true> elementValueRef;
				if (action == Action::POP_LEFT)
				{
					if (!listValue->PopLeft(elementValueRef, error))
						return Executor::Result::RUNTIME_ERROR;
				}
				else
				{
					if (!listValue->PopRight(elementValueRef, error))
						return Executor::Result::RUNTIME_ERROR;
				}
				if (!executor->PushValueOntoEvaluationStackTop(elementValueRef.Get(), error))
					return Executor::Result::RUNTIME_ERROR;
				if (virtualMachine->GetDebuggerTrap())
					virtualMachine->GetDebuggerTrap()->ValueChanged(listValue);
				break;
			}
			case Action::PUSH_LEFT:
			case Action::PUSH_RIGHT:
			{
				// Notice that we pop the pushed value, but leave the list value.
				GC::Reference<Value, true> elementValueRef;
				if (!executor->PopValueFromEvaluationStackTop(elementValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				Value* value = executor->StackTop(error);
				if (!value)
					return Executor::Result::RUNTIME_ERROR;
				ListValue* listValue = dynamic_cast<ListValue*>(value);
				if (!listValue)
				{
					error.Add("List instruction can only push elements to a list value.");
					return Executor::Result::RUNTIME_ERROR;
				}
				if (action == Action::PUSH_LEFT)
					listValue->PushLeft(elementValueRef.Get());
				else
					listValue->PushRight(elementValueRef.Get());
				if (virtualMachine->GetDebuggerTrap())
					virtualMachine->GetDebuggerTrap()->ValueChanged(listValue);
				break;
			}
			default:
			{
				error.Add(std::format("List instruction encountered unknown action {}.", action));
				return Executor::Result::RUNTIME_ERROR;
			}
		}

		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool ListInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		const AssemblyData::Entry* actionEntry = this->assemblyData->configMap.LookupPtr("action");
		if (!actionEntry)
		{
			error.Add(std::string(this->assemblyData->fileLocation) + "Assembly of list instruction failed because no action was given.");
			return false;
		}

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
					error.Add(std::string(this->assemblyData->fileLocation) + std::format("Did not recognize list instruction code: {}", actionEntry->code));
					return false;
				}
			}
		}

		programBufferLocation += 2;
		return true;
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
			detail += std::format("{}", actionEntry->code);
		return detail;
	}
#endif
}