#include "Executor.h"
#include "Instruction.h"
#include "VirtualMachine.h"
#include "Scope.h"

namespace Powder
{
	Executor::Executor(uint64_t programBufferLocation, Scope* currentScope)
	{
		this->programBufferLocation = programBufferLocation;
		this->currentScope = currentScope;
		this->evaluationStack = new std::vector<GCReference<Value>>();
	}

	/*virtual*/ Executor::~Executor()
	{
		while (this->PopScope())
		{
		}

		delete this->evaluationStack;
	}

	bool Executor::PushScope()
	{
		Scope* newScope = new Scope(this->currentScope);
		this->currentScope = newScope;
		return true;
	}

	bool Executor::PopScope()
	{
		Scope* containingScope = this->currentScope->GetContainingScope();
		if (!containingScope)
			return false;

		delete this->currentScope;
		this->currentScope = containingScope;
		return true;
	}

	bool Executor::LoadValue(const char* identifier)
	{
		Value* value = this->currentScope->LookupValue(identifier);
		if (!value)
			return false;

		this->evaluationStack->push_back(value);
		return true;
	}

	bool Executor::StoreValue(const char* identifier)
	{
		if (this->evaluationStack->size() == 0)
			return false;

		this->currentScope->StoreValue(identifier, (*this->evaluationStack)[this->evaluationStack->size() - 1]);
		this->evaluationStack->pop_back();
		return true;
	}

	/*virtual*/ Executor::Result Executor::Execute(uint8_t* programBuffer, uint64_t programBufferSize, VirtualMachine* virtualMachine)
	{
		while (this->programBufferLocation < programBufferSize)
		{
			Instruction* instruction = virtualMachine->LookupInstruction(programBuffer[this->programBufferLocation]);
			if (!instruction)
				break;

			Executor::Result result = instruction->Execute(programBuffer, programBufferSize, this->programBufferLocation, this, virtualMachine);
			if (result != Executor::Result::CONTINUE)
				return result;
		}

		return Result::HALT;
	}
}