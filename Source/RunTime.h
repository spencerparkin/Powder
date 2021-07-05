#pragma once

#include "Defines.h"
#include "HashMap.hpp"
#include "Scope.h"
#include <cstdint>
#include <string>

namespace Powder
{
	class MapValue;

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

			virtual uint8_t* CompileCode(const char* programCode, uint64_t& programBufferSize) = 0;
		};

		void ExecuteSourceCodeFile(const std::string& programSourceCodePath, Scope* scope = nullptr);
		void ExecuteSourceCode(const std::string& programSourceCode, const std::string& programSourceCodePath, Scope* scope = nullptr);

		typedef MapValue* (*GenerateFunctionMapFunc)();
		MapValue* LoadModuleFunctionMap(const std::string& moduleAbsolutePath);
		void UnloadAllModules(void);

	protected:

		CompilerInterface* compiler;

		GCReference<Scope> globalScope;

		typedef HashMap<void*> ModuleMap;
		ModuleMap moduleMap;
	};
}