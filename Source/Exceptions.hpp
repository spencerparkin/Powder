#pragma once

#include <string>

namespace Powder
{
	class Exception
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

	class RunTimeException : public Exception
	{
	public:
		RunTimeException(const std::string& errorMsg) : Exception(errorMsg)
		{
		}

		virtual ~RunTimeException()
		{
		}
	};

	class CompileTimeException : public Exception
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