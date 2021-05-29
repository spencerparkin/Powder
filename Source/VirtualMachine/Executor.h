#pragma once

#include <cinttypes>
#include <vector>
#include "Value.h"
#include "GCReference.hpp"

namespace Powder
{
	class VirtualMachine;
	class Scope;

	class POWDER_API Executor
	{
	public:
		Executor(uint64_t programBufferLocation, Scope* currentScope);
		virtual ~Executor();

		enum class Result
		{
			HALT,
			YIELD,
			CONTINUE
		};

		virtual Result Execute(uint8_t* programBuffer, uint64_t programBufferSize, VirtualMachine* virtualMachine);

		bool PushScope();
		bool PopScope();

		bool LoadValue(const char* identifier);
		bool StoreValue(const char* identifier);

	protected:

		uint64_t programBufferLocation;
		Scope* currentScope;
		std::vector<GCReference<Value>>* evaluationStack;
	};
}