#include "Executor.h"
#include "Instruction.h"
#include "VirtualMachine.h"
#include "Value.h"
#include "Scope.h"
#include "GarbageCollector.h"
#include "Exceptions.hpp"
#include "StringFormat.h"
#include "Executable.h"

namespace Powder
{
	Executor::Executor(uint64_t programBufferLocation, Scope* scope)
	{
		this->currentScope = scope;
		this->programBufferLocation = programBufferLocation;
		this->evaluationStack = new std::vector<GCReference<Value>>();
	}

	/*virtual*/ Executor::~Executor()
	{
		delete this->evaluationStack;
	}

	bool Executor::PushScope(Scope* scope /*= nullptr*/)
	{
		Scope* newScope = scope ? scope : new Scope();
		newScope->SetContainingScope(this->currentScope);
		this->currentScope = newScope;
		return true;
	}

	bool Executor::PopScope()
	{
		if (!this->currentScope->GetContainingScope())
			return false;

		Scope* containingScope = this->currentScope->GetContainingScope();
		this->currentScope->SetContainingScope(nullptr);
		this->currentScope = containingScope;
		return true;
	}

	void Executor::ReplaceCurrentScope(Scope* scope)
	{
		scope->SetContainingScope(this->currentScope->GetContainingScope());
		this->currentScope->SetContainingScope(nullptr);
		this->currentScope = scope;
	}

	/*virtual*/ Executor::Result Executor::Execute(const Executable* executable, VirtualMachine* virtualMachine)
	{
		while (this->programBufferLocation < executable->byteCodeBufferSize)
		{
			VirtualMachine::DebuggerTrap* debuggerTrap = virtualMachine->GetDebuggerTrap();
			if (debuggerTrap)
				if (debuggerTrap->TrapExecution(executable, this))
					return Executor::Result::HALT;

			uint8_t opCode = executable->byteCodeBuffer[this->programBufferLocation];
			Instruction* instruction = virtualMachine->LookupInstruction(opCode);
			if (!instruction)
				throw new RunTimeException(FormatString("Encountered unknown opcode 0x%04x at program location %04d.", opCode, this->programBufferLocation));

			Executor::Result result = (Executor::Result)instruction->Execute(executable, this->programBufferLocation, this, virtualMachine);
			if (result != Executor::Result::CONTINUE)
				return result;

			GarbageCollector::GC()->FreeObjects();
		}

		return Result::HALT;
	}

	void Executor::LoadAndPushValueOntoEvaluationStackTop(const char* identifier, void* debuggerTrap)
	{
		Value* value = this->currentScope->LookupValue(identifier, true);
		if (!value)
			throw new RunTimeException(FormatString("Failed to lookup identifier: %s", identifier));
		this->PushValueOntoEvaluationStackTop(value, false);
		if (debuggerTrap)
			((VirtualMachine::DebuggerTrap*)debuggerTrap)->ValueLoaded(identifier, value);
	}

	void Executor::StoreAndPopValueFromEvaluationStackTop(const char* identifier, void* debuggerTrap)
	{
		Value* value = this->PopValueFromEvaluationStackTop(true);
		this->currentScope->StoreValue(identifier, value);
		if (debuggerTrap)
			((VirtualMachine::DebuggerTrap*)debuggerTrap)->ValueStored(identifier, value);
		value->DecRef();
	}

	void Executor::PushValueOntoEvaluationStackTop(Value* value, bool decRefAfterPush)
	{
		this->evaluationStack->push_back(value);
		if (decRefAfterPush)
			value->DecRef();
	}

	Value* Executor::PopValueFromEvaluationStackTop(bool incRefBeforePop)
	{
		Value* value = nullptr;
		if (this->evaluationStack->size() == 0)
			throw new RunTimeException("Evaluation stack underflow!");
		value = (*this->evaluationStack)[this->evaluationStack->size() - 1];
		if (incRefBeforePop)
			value->IncRef();
		this->evaluationStack->pop_back();
		return value;
	}

	Value* Executor::StackTop()
	{
		return this->StackValue(0);
	}

	Value* Executor::StackValue(int32_t stackOffset)
	{
		int32_t i = signed(this->evaluationStack->size()) - 1 - stackOffset;
		if(0 <= i && i < (signed)this->evaluationStack->size())
			return (*this->evaluationStack)[i];
		throw new RunTimeException(FormatString("Stack at size %d cannot use offset %d.", this->evaluationStack->size(), i));
		return nullptr;
	}
}