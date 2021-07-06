#pragma once

#include <cinttypes>
#include "LinkedList.hpp"
#include "HashMap.hpp"

namespace Powder
{
	class Executor;
	class Executable;
	class RunTime;
	class Scope;

	// The goal here is to facilitate a basic procedural-style programming language.
	// Nothing dictates the syntax of that language here.  Rather, a compiler will
	// have to be written that targets this virtual machine.  The only, perhaps,
	// atypical feature provided here, is that multiple points of execution can be
	// managed by the VM.  While they do not run simultaneously, they do run in a
	// cooperative fasion (like fibers), which simulates some notion of concurrency
	// in the execution of the given program.
	class POWDER_API VirtualMachine
	{
	public:
		VirtualMachine(RunTime* runTime);
		virtual ~VirtualMachine();

		void ExecuteByteCode(const Executable* executable, Scope* scope);
		void CreateExecutorAtLocation(uint64_t programBufferLocation, Scope* scope);

		RunTime* GetRunTime() { return this->runTime; }

	protected:

		RunTime* runTime;

		typedef LinkedList<Executor*> ExecutorList;
		ExecutorList executorList;
	};
}