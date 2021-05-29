#pragma once

#include "Defines.h"
#include <cstdint>

namespace Powder
{
	class POWDER_API Compiler
	{
	public:
		Compiler();
		virtual ~Compiler();

		uint8_t* CompileCode(const char* programCode, uint64_t& programBufferSize);
	};
}