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

	void Executor::AbsorbScope(Scope* scope)
	{
		this->currentScope->Absorb(scope);
	}

	/*virtual*/ Executor::Result Executor::Execute(const Executable* executable, VirtualMachine* virtualMachine)
	{
		while (this->programBufferLocation < executable->byteCodeBufferSize)
		{
			uint8_t opCode = executable->byteCodeBuffer[this->programBufferLocation];
			Instruction* instruction = virtualMachine->LookupInstruction(opCode);
			if (!instruction)
				throw new RunTimeException(FormatString("Encountered unknown opcode 0x%04x at program location %04d.", opCode, this->programBufferLocation));

			Executor::Result result = (Executor::Result)instruction->Execute(executable, this->programBufferLocation, this, virtualMachine);
			if (result != Executor::Result::CONTINUE)
				return result;

#if false
			// This is super slow, but is also a way to test that the GC system doesn't
			// delete stuff that it shouldn't delete.
			GarbageCollector::GC()->FullPurge();
#else
			// TODO: This will delete objects as the program runs, but the amount of
			//       allocated memory can still steadily grow as time goes on.  Should
			//       there be some sort of dynamic system in place to call the GC more
			//       or less to keep our memory usage within a certain limit?
			GarbageCollector::GC()->Run();
			GarbageCollector::GC()->Run();
			GarbageCollector::GC()->Run();
			GarbageCollector::GC()->Run();
#endif
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