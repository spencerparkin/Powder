#pragma once

#if defined POWDER_API_IMPORT
#	define POWDER_API		__declspec(dllimport)
#elif defined POWDER_API_EXPORT
#	define POWDER_API		__declspec(dllexport)
#else
#	define POWDER_API
#endif

namespace Powder
{
}