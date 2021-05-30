#pragma once

#include "Defines.h"
#include <cinttypes>

namespace Powder
{
	class VirtualMachine;
	class Scope;

	class POWDER_API Executor
	{
	public:
		Executor(uint64_t programBufferLocation);
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

		Scope* GetCurrentScope() { return this->currentScope; }

	protected:

		uint64_t programBufferLocation;
		Scope* currentScope;
	};
}