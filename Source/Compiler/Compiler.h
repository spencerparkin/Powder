#pragma once

#include "Defines.h"
#include "VirtualMachine.h"
#include <cstdint>

namespace Powder
{
	class POWDER_API Compiler : public VirtualMachine::CompilerInterface
	{
	public:
		Compiler();
		virtual ~Compiler();

		virtual Executable* CompileCode(const char* programSourceCode) override;

		bool generateDebugInfo;
	};

	extern Compiler theDefaultCompiler;
}