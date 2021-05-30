#include "Executor.h"
#include "Instruction.h"
#include "VirtualMachine.h"
#include "Value.h"
#include "Scope.h"

namespace Powder
{
	Executor::Executor(uint64_t programBufferLocation)
	{
		this->programBufferLocation = programBufferLocation;
		this->currentScope = nullptr;
	}

	/*virtual*/ Executor::~Executor()
	{
		while (this->PopScope())
		{
		}
	}

	bool Executor::PushScope()
	{
		Scope* newScope = new Scope(this->currentScope);
		this->currentScope = newScope;
		return true;
	}

	bool Executor::PopScope()
	{
		if (!this->currentScope)
			return false;

		Scope* containingScope = this->currentScope->GetContainingScope();
		delete this->currentScope;
		this->currentScope = containingScope;
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