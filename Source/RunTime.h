#pragma once

#include "Defines.h"
#include "HashMap.hpp"
#include "Scope.h"
#include <cstdint>
#include <string>

namespace Powder
{
	class MapValue;
	class Executable;
	class Instruction;

	// Note that while no specific compiler is dictated here, we do require access
	// to a general compiler interface as part of the code re-usability feature.
	class POWDER_API RunTime
	{
	public:
		class CompilerInterface;

		RunTime(CompilerInterface* compiler);
		virtual ~RunTime();

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

		CompilerInterface* compiler;

		GCReference<Scope> globalScope;

		typedef HashMap<void*> ModuleMap;
		ModuleMap moduleMap;

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

		typedef HashMap<Instruction*> InstructionMap;
		InstructionMap instructionMap;
	};
}