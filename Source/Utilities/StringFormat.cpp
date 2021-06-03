#include "StringFormat.h"

namespace Powder
{
	// It took them (whoever they are) until C++20 to provide basic string formatting to the C++ language's standard C++ library.
	// But since I don't have C++20 yet, here we go...
	std::string FormatString(const char* formatBuffer, ...)
	{
		::va_list argList;
		va_start(argList, formatBuffer);
		char formattedStringBuffer[512];
		::vsprintf_s(formattedStringBuffer, sizeof(formattedStringBuffer), formatBuffer, argList);
		va_end(argList);

		std::string formattedStr = formattedStringBuffer;
		return formattedStr;
	}
}