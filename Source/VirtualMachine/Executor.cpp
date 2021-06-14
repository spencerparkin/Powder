#include "Executor.h"
#include "Instruction.h"
#include "VirtualMachine.h"
#include "Value.h"
#include "Scope.h"
#include "GarbageCollector.h"
#include "Exceptions.hpp"
#include "StringFormat.h"

namespace Powder
{
	Executor::Executor(uint64_t programBufferLocation, Executor* forkOrigin)
	{
		if (forkOrigin)
			this->currentScope = forkOrigin->currentScope;
		else
			this->currentScope = new Scope();

		this->programBufferLocation = programBufferLocation;
		this->evaluationStack = new std::vector<GCReference<Value>>();
	}

	/*virtual*/ Executor::~Executor()
	{
		delete this->evaluationStack;
	}

	bool Executor::PushScope()
	{
		Scope* newScope = new Scope();
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

	/*virtual*/ Executor::Result Executor::Execute(uint8_t* programBuffer, uint64_t programBufferSize, VirtualMachine* virtualMachine)
	{
		while (this->programBufferLocation < programBufferSize)
		{
			uint8_t opCode = programBuffer[this->programBufferLocation];
			Instruction* instruction = virtualMachine->LookupInstruction(opCode);
			if (!instruction)
				throw new RunTimeException(FormatString("Encountered unknown opcode 0x%04x", opCode));

			Executor::Result result = (Executor::Result)instruction->Execute(programBuffer, programBufferSize, this->programBufferLocation, this, virtualMachine);
			if (result != Executor::Result::CONTINUE)
				return result;

			// TODO: Run garbage collector here periodically, but once every tick might be too much.
		}

		return Result::HALT;
	}

	void Executor::LoadAndPushValueOntoEvaluationStackTop(const char* identifier)
	{
		Value* value = this->currentScope->LookupValue(identifier, true);
		if (!value)
			throw new RunTimeException(FormatString("Failed to lookup identifier: %s", identifier));

		return this->PushValueOntoEvaluationStackTop(value);
	}

	void Executor::StoreAndPopValueFromEvaluationStackTop(const char* identifier)
	{
		Value* value = this->PopValueFromEvaluationStackTop();
		this->currentScope->StoreValue(identifier, value);
	}

	void Executor::PushValueOntoEvaluationStackTop(Value* value)
	{
		this->evaluationStack->push_back(value);
	}

	Value* Executor::PopValueFromEvaluationStackTop()
	{
		Value* value = nullptr;
		if (this->evaluationStack->size() == 0)
			throw new RunTimeException("Evaluation stack underflow!");
		value = (*this->evaluationStack)[this->evaluationStack->size() - 1];
		this->evaluationStack->pop_back();
		return value;
	}

	Value* Executor::StackTop()
	{
		if (this->evaluationStack->size() == 0)
			throw new RunTimeException("No stack top!");
		Value* value = (*this->evaluationStack)[this->evaluationStack->size() - 1];
		return value;
	}
}