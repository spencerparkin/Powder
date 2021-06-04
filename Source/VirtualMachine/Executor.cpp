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
	}

	/*virtual*/ Executor::~Executor()
	{
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
}