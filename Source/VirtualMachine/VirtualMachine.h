#pragma once

#include <cinttypes>
#include <vector>
#include "GCReference.hpp"
#include "LinkedList.hpp"
#include "HashMap.hpp"

namespace Powder
{
	class Executor;
	class Executable;
	class RunTime;
	class Scope;
	class MapValue;
	class Instruction;

	// The goal here is to facilitate a basic procedural-style programming language.
	// Nothing dictates the syntax of that language here.  Rather, a compiler will
	// have to be written that targets this virtual machine.  The only, perhaps,
	// atypical feature provided here, is that multiple points of execution can be
	// managed by the VM.  While they do not run simultaneously, they do run in a
	// cooperative fasion (like fibers), which simulates some notion of concurrency
	// in the execution of the given program.  Note that while no specific compiler
	// is dictated here, we do require access to a general compiler interface as
	// part of the code re-usability feature.
	class POWDER_API VirtualMachine
	{
	public:
		class CompilerInterface;
		class DebuggerInterface;

		VirtualMachine(CompilerInterface* compiler, DebuggerInterface* debugger);
		virtual ~VirtualMachine();

		void ExecuteByteCode(const Executable* executable, Scope* scope);
		void CreateExecutorAtLocation(uint64_t programBufferLocation, Scope* scope);

		class POWDER_API CompilerInterface
		{
		public:
			CompilerInterface() {}
			virtual ~CompilerInterface() {}

			virtual Executable* CompileCode(const char* programSourceCode) = 0;
		};

		class POWDER_API DebuggerInterface
		{
		public:
			DebuggerInterface() {}
			virtual ~DebuggerInterface() {}

			virtual void TrapExecution(const Executable* executable, Executor* executor) = 0;
		};

		void ExecuteSourceCodeFile(const std::string& programSourceCodePath, Scope* scope = nullptr);
		void ExecuteSourceCode(const std::string& programSourceCode, const std::string& programSourceCodePath, Scope* scope = nullptr);

		typedef MapValue* (*GenerateFunctionMapFunc)();
		MapValue* LoadModuleFunctionMap(const std::string& moduleAbsolutePath);
		void UnloadAllModules(void);

		Instruction* LookupInstruction(uint8_t programOpCode);

		DebuggerInterface* GetDebugger() { return this->debugger; }
		void SetDebugger(DebuggerInterface* debugger) { this->debugger = debugger; }

	protected:

		typedef LinkedList<Executor*> ExecutorList;
		typedef HashMap<void*> ModuleMap;
		typedef HashMap<Instruction*> InstructionMap;

		CompilerInterface* compiler;
		DebuggerInterface* debugger;
		GCReference<Scope> globalScope;
		ModuleMap moduleMap;
		InstructionMap instructionMap;
		std::vector<ExecutorList*>* executorListStack;

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
	};
}