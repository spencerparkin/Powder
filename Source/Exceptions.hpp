#pragma once

#include "Defines.h"
#include "StringFormat.h"
#include "Lexer.h"
#include <string>
#include <format>

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
		CompileTimeException(const std::string& errorMsg, const ParseParty::Lexer::FileLocation* fileLocation = nullptr) : Exception(errorMsg)
		{
			if (fileLocation)
				this->fileLocation = *fileLocation;
			else
				this->fileLocation = ParseParty::Lexer::FileLocation{ -1, -1 };
		}

		virtual ~CompileTimeException()
		{
		}

		virtual std::string GetErrorMessage() override
		{
			if (this->fileLocation.line >= 0)
				return std::format("(Ln {}, Col {}): Compile-time error: ", this->fileLocation.line, this->fileLocation.column) + *this->errorMsg;
			else
				return "Compile-time error: " + *this->errorMsg;
		}

		ParseParty::Lexer::FileLocation fileLocation;
	};
}