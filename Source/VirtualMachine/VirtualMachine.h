#pragma once

#include <cinttypes>
#include <vector>
#include "Reference.h"
#include "LinkedList.hpp"
#include "HashMap.hpp"
#include "Value.h"
#include "Scope.h"

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
		VirtualMachine();
		virtual ~VirtualMachine();

		void ExecuteByteCode(const Executable* executable, Scope* scope);
		void CreateExecutorAtLocation(uint64_t programBufferLocation, Scope* scope);

		class POWDER_API CompilerInterface
		{
		public:
			CompilerInterface();
			virtual ~CompilerInterface();

			virtual Executable* CompileCode(const char* programSourceCode) = 0;
		};

		class POWDER_API IODevice
		{
		public:
			IODevice();
			virtual ~IODevice();

			virtual void InputString(std::string& str);
			virtual void OutputString(const std::string& str);
		};

		class POWDER_API DebuggerTrap
		{
		public:
			DebuggerTrap();
			virtual ~DebuggerTrap();

			virtual bool TrapExecution(const Executable* executable, Executor* executor) = 0;
			virtual void ValueChanged(const Value* value) = 0;
			virtual void ValueStored(const char* name, const Value* value) = 0;
			virtual void ValueLoaded(const char* name, const Value* value) = 0;
		};

		void ExecuteSourceCodeFile(const std::string& programSourceCodePath, Scope* scope = nullptr);
		void ExecuteSourceCode(const std::string& programSourceCode, const std::string& programSourceCodePath, Scope* scope = nullptr);

		typedef MapValue* (*GenerateFunctionMapFunc)();
		MapValue* LoadModuleFunctionMap(const std::string& moduleAbsolutePath);
		void UnloadAllModules(void);

		Instruction* LookupInstruction(uint8_t programOpCode);

		void SetCompiler(CompilerInterface* compiler) { this->compiler = compiler; }
		CompilerInterface* GetCompiler() { return this->compiler; }

		void SetIODevice(IODevice* ioDevice) { this->ioDevice = ioDevice; }
		IODevice* GetIODevice() { return this->ioDevice; }

		void SetDebuggerTrap(DebuggerTrap* debuggerTrap) { this->debuggerTrap = debuggerTrap; }
		DebuggerTrap* GetDebuggerTrap() { return this->debuggerTrap; }

		void GetAllCurrentScopes(std::vector<Scope*>& scopeArray);

	protected:

		typedef LinkedList<Executor*> ExecutorList;
		typedef HashMap<void*> ModuleMap;
		typedef HashMap<Instruction*> InstructionMap;

		CompilerInterface* compiler;
		IODevice* ioDevice;
		DebuggerTrap* debuggerTrap;
		GC::Reference<Scope, true> globalScopeRef;
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