#pragma once

#include "Defines.h"
#include <string>
#include <stdarg.h>

namespace Powder
{
	POWDER_API std::string FormatString(const char* formatBuffer, ...);
}