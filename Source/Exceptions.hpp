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
			this->errorMsg = new std::string;
			*this->errorMsg = errorMsg;
		}

		virtual ~Exception()
		{
			delete this->errorMsg;
		}

		virtual std::string GetErrorMessage()
		{
			return *this->errorMsg;
		}

		std::string* errorMsg;
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

		virtual std::string GetErrorMessage() override
		{
			return "Run-time error: " + *this->errorMsg;
		}
	};

	class POWDER_API CompileTimeException : public Exception
	{
	public:
		CompileTimeException(const std::string& errorMsg, uint16_t lineNumber = -1) : Exception(errorMsg)
		{
			this->lineNumber = lineNumber;
		}

		virtual ~CompileTimeException()
		{
		}

		virtual std::string GetErrorMessage() override
		{
			// TODO: Format line number in with this message.
			return "Compile-time error: " + *this->errorMsg;
		}

		uint16_t lineNumber;
	};
}