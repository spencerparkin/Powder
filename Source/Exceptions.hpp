#pragma once

#include "Defines.h"
#include "StringFormat.h"
#include <string>

namespace Powder
{
	struct FileLocation
	{
		uint16_t lineNumber;
		uint16_t columnNumber;
	};

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
		CompileTimeException(const std::string& errorMsg, const FileLocation* fileLocation = nullptr) : Exception(errorMsg)
		{
			if (fileLocation)
				this->fileLocation = *fileLocation;
			else
			{
				this->fileLocation.lineNumber = -1;
				this->fileLocation.columnNumber = -1;
			}
		}

		virtual ~CompileTimeException()
		{
		}

		virtual std::string GetErrorMessage() override
		{
			std::string formattedErrorMsg;
			formattedErrorMsg = "Compile-time error...\n";
			if (this->fileLocation.lineNumber != -1)
			{
				formattedErrorMsg += FormatString("Line number: %d\n", this->fileLocation.lineNumber);
				if (this->fileLocation.columnNumber != -1)
					formattedErrorMsg += FormatString("Column number: %d\n", this->fileLocation.columnNumber);
			}
			formattedErrorMsg += *this->errorMsg;
			return formattedErrorMsg;
		}

		FileLocation fileLocation;
	};
}