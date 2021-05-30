#pragma once

#include <cinttypes>
#include <list>
#include "HashMap.hpp"

namespace Powder
{
	class Executor;
	class Instruction;
	class Scope;

	// The goal here is to facilitate a basic procedural-style programming language.
	// Nothing dictates the syntax of that language here.  Rather, a compiler will
	// have to be written that targets this virtual machine.  The only, perhaps,
	// atypical feature provided here, is that multiple points of execution can be
	// managed by the VM.  While they do not run simultaneously, they do run in a
	// cooperative fasion, which simulates some notion of concurrency in the execution
	// of the given program.
	class POWDER_API VirtualMachine
	{
	public:
		VirtualMachine();
		virtual ~VirtualMachine();

		virtual void Execute(uint8_t* programBuffer, uint64_t programBufferSize);

		void CreateExecutorAtLocation(uint64_t programBufferLocation);

		Instruction* LookupInstruction(uint8_t programOpCode);

		template<typename T>
		void RegisterInstruction()
		{
			char opCodeStr[2] = { (char)T::OpCode(), '\0' };
			if (!this->instructionMap->Lookup(opCodeStr))
			{
				T* instruction = new T();
				this->instructionMap->Insert(opCodeStr, instruction);
			}
		}

	protected:

		typedef std::list<Executor*> ExecutorList;
		ExecutorList* executorList;

		typedef HashMap<Instruction*> InstructionMap;
		InstructionMap* instructionMap;
	};
}