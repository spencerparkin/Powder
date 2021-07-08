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

	// TODO: How would we support debugging of programs that run on the VM?
	//       Firstly, this is different than embedding or extending the VM.
	//       A debugger should be able to attach to the VM, no matter where
	//       it's embedded or extended.  I'm thinking this might mean some
	//       sort of socket communication protocol where the VM acts as server,
	//       and a connected client issues commands.  Also, a .pwx file may
	//       or may not have debug info appended, but it should be debuggable
	//       in either case.  With debug info, it should be possible to view
	//       the source code in context of where the VM is executing the program.

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

		VirtualMachine(CompilerInterface* compiler);
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

		void ExecuteSourceCodeFile(const std::string& programSourceCodePath, Scope* scope = nullptr);
		void ExecuteSourceCode(const std::string& programSourceCode, const std::string& programSourceCodePath, Scope* scope = nullptr);

		typedef MapValue* (*GenerateFunctionMapFunc)();
		MapValue* LoadModuleFunctionMap(const std::string& moduleAbsolutePath);
		void UnloadAllModules(void);

		Instruction* LookupInstruction(uint8_t programOpCode);

	protected:

		typedef LinkedList<Executor*> ExecutorList;
		typedef HashMap<void*> ModuleMap;
		typedef HashMap<Instruction*> InstructionMap;

		CompilerInterface* compiler;
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