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

	/*virtual*/ Executor::Result Executor::Execute(const Executable* executable, VirtualMachine* virtualMachine)
	{
		uint32_t gcRunRequency = 10;
		uint32_t tickCount = 0;

		while (this->programBufferLocation < executable->byteCodeBufferSize)
		{
			uint8_t opCode = executable->byteCodeBuffer[this->programBufferLocation];
			Instruction* instruction = virtualMachine->LookupInstruction(opCode);
			if (!instruction)
				throw new RunTimeException(FormatString("Encountered unknown opcode 0x%04x", opCode));

			Executor::Result result = (Executor::Result)instruction->Execute(executable, this->programBufferLocation, this, virtualMachine);
			if (result != Executor::Result::CONTINUE)
				return result;

			// TODO: A single run of the GC is still potentially too slow, because if, say, there
			//       is a list of several thousand strings, we may try to go iterate that whole
			//       list right here and now.  We might be able to fix this by turning the run
			//       into an incremental state machine so that a BFS can be resumed where it left
			//       off.  The tricky part is knowing that the spanning tree we're trying to find
			//       could be growing while we're trying to find it.  We must convince ourselves
			//       that this isn't a problem with whatever alogirhtm we come up with.
			if (tickCount++ % gcRunRequency == 0)
				GarbageCollector::GC()->Run();
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