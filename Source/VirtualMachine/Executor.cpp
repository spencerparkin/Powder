#include "Executor.h"
#include "Instruction.h"
#include "VirtualMachine.h"
#include "Value.h"
#include "Scope.h"
#include "GarbageCollector.h"
#include "Error.h"
#include "Executable.h"

namespace Powder
{
	Executor::Executor(uint64_t programBufferLocation, const Executable* executable, Scope* scope)
	{
		this->currentScopeRef.Set(scope);
		this->executableRef.Set(const_cast<Executable*>(executable));
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

		GC::Reference<Scope, true> containingScopeRef(this->currentScopeRef.Get()->GetContainingScope());
		this->currentScopeRef.Get()->SetContainingScope(nullptr);
		this->currentScopeRef.Set(containingScopeRef.Get());
		return true;
	}

	void Executor::ReplaceCurrentScope(Scope* scope)
	{
		scope->SetContainingScope(this->currentScopeRef.Get()->GetContainingScope());
		this->currentScopeRef.Get()->SetContainingScope(nullptr);
		this->currentScopeRef.Set(scope);
	}

	/*virtual*/ Executor::Result Executor::Execute(VirtualMachine* virtualMachine, Error& error)
	{
		if (!this->executableRef.Get())
		{
			error.Add("Can't run null executable.");
			return Executor::Result::RUNTIME_ERROR;
		}

		while (this->programBufferLocation < this->executableRef.Get()->byteCodeBufferSize)
		{
			VirtualMachine::DebuggerTrap* debuggerTrap = virtualMachine->GetDebuggerTrap();
			if (debuggerTrap)
				if (debuggerTrap->TrapExecution(this->executableRef.Get(), this))
					return Executor::Result::HALT;

			uint8_t opCode = this->executableRef.Get()->byteCodeBuffer[this->programBufferLocation];
			Instruction* instruction = virtualMachine->LookupInstruction(opCode);
			if (!instruction)
			{
				error.Add(std::format("Encountered unknown opcode {} at program location {}.", opCode, this->programBufferLocation));
				return Result::RUNTIME_ERROR;
			}

			Executor::Result result = (Executor::Result)instruction->Execute(this->executableRef, this->programBufferLocation, this, virtualMachine, error);
			if (result != Executor::Result::CONTINUE)
				return result;
		}

		return Result::HALT;
	}

	bool Executor::LoadAndPushValueOntoEvaluationStackTop(const char* identifier, Error& error, void* debuggerTrap)
	{
		Value* value = this->currentScopeRef.Get()->LookupValue(identifier, true);
		if (!value)
		{
			error.Add(std::format("Failed to lookup identifier: {}", identifier));
			return false;
		}

		if (!this->PushValueOntoEvaluationStackTop(value, error))
			return false;

		if (debuggerTrap)
			((VirtualMachine::DebuggerTrap*)debuggerTrap)->ValueLoaded(identifier, value);

		return true;
	}

	bool Executor::StoreAndPopValueFromEvaluationStackTop(const char* identifier, Error& error, void* debuggerTrap)
	{
		GC::Reference<Value, true> valueRef;
		if (!this->PopValueFromEvaluationStackTop(valueRef, error))
			return false;

		this->currentScopeRef.Get()->StoreValue(identifier, valueRef.Get());
		
		if (debuggerTrap)
			((VirtualMachine::DebuggerTrap*)debuggerTrap)->ValueStored(identifier, valueRef.Get());

		return true;
	}

	bool Executor::PushValueOntoEvaluationStackTop(Value* value, Error& error)
	{
		this->evaluationStack->push_back(value);
		return true;
	}

	bool Executor::PopValueFromEvaluationStackTop(GC::Reference<Value, true>& valueRef, Error& error)
	{
		if (this->evaluationStack->size() == 0)
		{
			error.Add("Evaluation stack underflow!");
			return false;
		}

		valueRef.Set((*this->evaluationStack)[this->evaluationStack->size() - 1].Get());
		this->evaluationStack->pop_back();
		return true;
	}

	Value* Executor::StackTop(Error& error)
	{
		return this->StackValue(0, &error);
	}

	Value* Executor::StackValue(int32_t stackOffset, Error* error)
	{
		int32_t i = signed(this->evaluationStack->size()) - 1 - stackOffset;
		if(0 <= i && i < (signed)this->evaluationStack->size())
			return (*this->evaluationStack)[i].Get();
		
		if (error)
			error->Add(std::format("Stack at size {} cannot use offset {}.", this->evaluationStack->size(), i));

		return nullptr;
	}

	uint32_t Executor::StackSize() const
	{
		return this->evaluationStack->size();
	}
}