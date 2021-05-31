#pragma once

#include "Defines.h"
#include <string>

namespace Powder
{
	class POWDER_API Exception
	{
	public:
		Exception(const std::string& errorMsg)
		{
			this->errorMsg = errorMsg;
		}

		virtual ~Exception()
		{
		}

		std::string errorMsg;
	};

	class POWDER_API RunTimeException : public Exception
	{
	public:
		RunTimeException(const std::string& errorMsg) : Exception(errorMsg)
		{
		}

		virtual ~RunTimeException()
		{
		}
	};

	class POWDER_API CompileTimeException : public Exception
	{
	public:
		CompileTimeException(const std::string& errorMsg) : Exception(errorMsg)
		{
		}

		virtual ~CompileTimeException()
		{
		}
	};
}