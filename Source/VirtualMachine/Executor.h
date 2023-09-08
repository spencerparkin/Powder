#pragma once

#include "Defines.h"
#include "Value.h"
#include "Scope.h"
#include "Reference.h"
#include <cinttypes>

namespace Powder
{
	class Error;
	class VirtualMachine;
	class Scope;
	class Executable;

	class POWDER_API Executor
	{
	public:
		Executor(uint64_t programBufferLocation, Scope* scope);
		virtual ~Executor();

		enum Result
		{
			HALT,
			YIELD,
			CONTINUE,
			RUNTIME_ERROR
		};

		virtual Result Execute(const Executable* executable, VirtualMachine* virtualMachine, Error& error);

		bool PushScope();
		bool PopScope();

		Scope* GetCurrentScope() { return this->currentScopeRef.Get(); }
		uint64_t GetProgramBufferLocation() { return this->programBufferLocation; }

		void ReplaceCurrentScope(Scope* scope);

		bool LoadAndPushValueOntoEvaluationStackTop(const char* identifier, Error& error, void* debuggerTrap);
		bool StoreAndPopValueFromEvaluationStackTop(const char* identifier, Error& error, void* debuggerTrap);

		bool PushValueOntoEvaluationStackTop(Value* value, Error& error);
		bool PopValueFromEvaluationStackTop(GC::Reference<Value, true>& valueRef, Error& error);

		Value* StackTop(Error& error);
		Value* StackValue(int32_t stackOffset, Error& error);	// This is relative to the top of the stack.

	protected:

		uint64_t programBufferLocation;
		GC::Reference<Scope, true> currentScopeRef;
		std::vector<GC::Reference<Value, true>>* evaluationStack;
	};
}