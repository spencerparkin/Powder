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

	class POWDER_API Executor
	{
	public:
		Executor(uint64_t programBufferLocation, Executor* forkOrigin);
		virtual ~Executor();

		enum Result
		{
			HALT,
			YIELD,
			CONTINUE
		};

		virtual Result Execute(uint8_t* programBuffer, uint64_t programBufferSize, VirtualMachine* virtualMachine);

		bool PushScope();
		bool PopScope();

		Scope* GetCurrentScope() { return this->currentScope.Ptr(); }

	protected:

		uint64_t programBufferLocation;
		GCReference<Scope> currentScope;
	};
}