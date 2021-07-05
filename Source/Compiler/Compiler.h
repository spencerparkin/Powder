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

		virtual uint8_t* CompileCode(const char* programCode, uint64_t& programBufferSize) override;
	};
}