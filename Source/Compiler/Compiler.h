#pragma once

#include "Defines.h"
#include "RunTime.h"
#include <cstdint>

namespace Powder
{
	class POWDER_API Compiler : public RunTime::CompilerInterface
	{
	public:
		Compiler();
		virtual ~Compiler();

		virtual Executable* CompileCode(const char* programSourceCode) override;
	};
}