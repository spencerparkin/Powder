#pragma once

#include "Defines.h"
#include "Value.h"
#include "Scope.h"
#include "Executable.h"
#include "Reference.h"
#include <cinttypes>

namespace Powder
{
	class Error;
	class VirtualMachine;
	class Scope;
	class Executable;

	// A better name for this might have been "Thread" or "Fiber".
	class POWDER_API Executor
	{
	public:
		Executor(uint64_t programBufferLocation, const Executable* executable, Scope* scope);
		virtual ~Executor();

		enum Result
		{
			HALT,
			YIELD,
			CONTINUE,
			RUNTIME_ERROR
		};

		virtual Result Execute(VirtualMachine* virtualMachine, Error& error);

		bool PushScope();
		bool PopScope();

		Scope* GetCurrentScope() { return this->currentScopeRef.Get(); }
		uint64_t GetProgramBufferLocation() { return this->programBufferLocation; }
		const Executable* GetExecutable() { return this->executableRef.Get(); }

		void ReplaceCurrentScope(Scope* scope);

		bool LoadAndPushValueOntoEvaluationStackTop(const char* identifier, Error& error, void* debuggerTrap);
		bool StoreAndPopValueFromEvaluationStackTop(const char* identifier, Error& error, void* debuggerTrap);

		bool PushValueOntoEvaluationStackTop(Value* value, Error& error);
		bool PopValueFromEvaluationStackTop(GC::Reference<Value, true>& valueRef, Error& error);

		Value* StackTop(Error& error);
		Value* StackValue(int32_t stackOffset, Error* error);	// This is relative to the top of the stack.
		uint32_t StackSize() const;

	protected:

		uint64_t programBufferLocation;
		GC::Reference<const Executable, true> executableRef;
		GC::Reference<Scope, true> currentScopeRef;
		std::vector<GC::Reference<Value, true>>* evaluationStack;
	};
}