#pragma once

#include "Defines.h"
#include "Value.h"
#include "Scope.h"
#include <GCReference.hpp>
#include <cinttypes>

namespace Powder
{
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
			CONTINUE
		};

		virtual Result Execute(const Executable* executable, VirtualMachine* virtualMachine);

		bool PushScope(Scope* scope = nullptr);
		bool PopScope();

		Scope* GetCurrentScope() { return this->currentScope.Ptr(); }
		uint64_t GetProgramBufferLocation() { return this->programBufferLocation; }

		void ReplaceCurrentScope(Scope* scope);

		void LoadAndPushValueOntoEvaluationStackTop(const char* identifier, void* debuggerTrap);
		void StoreAndPopValueFromEvaluationStackTop(const char* identifier, void* debuggerTrap);

		void PushValueOntoEvaluationStackTop(Value* value, bool decRefAfterPush);
		Value* PopValueFromEvaluationStackTop(bool incRefBeforePop);

		Value* StackTop();
		Value* StackValue(int32_t stackOffset);	// This is relative to the top of the stack.

	protected:

		uint64_t programBufferLocation;
		GCReference<Scope> currentScope;
		std::vector<GCReference<Value>>* evaluationStack;
	};
}