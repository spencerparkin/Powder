#pragma once

#include <cinttypes>
#include "LinkedList.hpp"
#include "HashMap.hpp"

namespace Powder
{
	class Executor;
	class Instruction;
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

		void ExecuteByteCode(uint8_t* programBuffer, uint64_t programBufferSize, Scope* scope);
		void CreateExecutorAtLocation(uint64_t programBufferLocation, Scope* scope);
		Instruction* LookupInstruction(uint8_t programOpCode);

		RunTime* runTime;

	protected:

		template<typename T>
		void RegisterInstruction()
		{
			T* instruction = new T();
			char opCodeStr[2] = { (char)instruction->OpCode(), '\0' };
			if (this->instructionMap.Lookup(opCodeStr))
				delete instruction;
			else
				this->instructionMap.Insert(opCodeStr, instruction);
		}

		typedef LinkedList<Executor*> ExecutorList;
		ExecutorList executorList;

		typedef HashMap<Instruction*> InstructionMap;
		InstructionMap instructionMap;
	};
}