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
		this->currentScopeRef.Set(scope);
		this->programBufferLocation = programBufferLocation;
		this->evaluationStack = new std::vector<GC::Reference<Value, true>>();
	}

	/*virtual*/ Executor::~Executor()
	{
		delete this->evaluationStack;
	}

	bool Executor::PushScope()
	{
		Scope* newScope = new Scope();
		newScope->SetContainingScope(this->currentScopeRef.Get());
		this->currentScopeRef.Set(newScope);
		return true;
	}

	bool Executor::PopScope()
	{
		if (!this->currentScopeRef.Get()->GetContainingScope())
			return false;

		Scope* containingScope = this->currentScopeRef.Get()->GetContainingScope();
		this->currentScopeRef.Get()->SetContainingScope(nullptr);
		this->currentScopeRef.Set(containingScope);
		return true;
	}

	void Executor::ReplaceCurrentScope(Scope* scope)
	{
		scope->SetContainingScope(this->currentScopeRef.Get()->GetContainingScope());
		this->currentScopeRef.Get()->SetContainingScope(nullptr);
		this->currentScopeRef.Set(scope);
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
		}

		return Result::HALT;
	}

	void Executor::LoadAndPushValueOntoEvaluationStackTop(const char* identifier, void* debuggerTrap)
	{
		Value* value = this->currentScopeRef.Get()->LookupValue(identifier, true);
		if (!value)
			throw new RunTimeException(FormatString("Failed to lookup identifier: %s", identifier));
		this->PushValueOntoEvaluationStackTop(value);
		if (debuggerTrap)
			((VirtualMachine::DebuggerTrap*)debuggerTrap)->ValueLoaded(identifier, value);
	}

	void Executor::StoreAndPopValueFromEvaluationStackTop(const char* identifier, void* debuggerTrap)
	{
		GC::Reference<Value, true> valueRef;
		this->PopValueFromEvaluationStackTop(valueRef);
		this->currentScopeRef.Get()->StoreValue(identifier, valueRef.Get());
		if (debuggerTrap)
			((VirtualMachine::DebuggerTrap*)debuggerTrap)->ValueStored(identifier, valueRef.Get());
	}

	void Executor::PushValueOntoEvaluationStackTop(Value* value)
	{
		this->evaluationStack->push_back(value);
	}

	void Executor::PopValueFromEvaluationStackTop(GC::Reference<Value, true>& valueRef)
	{
		if (this->evaluationStack->size() == 0)
			throw new RunTimeException("Evaluation stack underflow!");
		valueRef.Set((*this->evaluationStack)[this->evaluationStack->size() - 1].Get());
		this->evaluationStack->pop_back();
	}

	Value* Executor::StackTop()
	{
		return this->StackValue(0);
	}

	Value* Executor::StackValue(int32_t stackOffset)
	{
		int32_t i = signed(this->evaluationStack->size()) - 1 - stackOffset;
		if(0 <= i && i < (signed)this->evaluationStack->size())
			return (*this->evaluationStack)[i].Get();
		throw new RunTimeException(FormatString("Stack at size %d cannot use offset %d.", this->evaluationStack->size(), i));
		return nullptr;
	}
}