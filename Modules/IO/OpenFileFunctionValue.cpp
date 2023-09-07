#include "OpenFileFunctionValue.h"
#include "ListValue.h"
#include "StringFormat.h"
#include "FileValue.h"
#include "UndefinedValue.h"
#include <filesystem>

OpenFileFunctionValue::OpenFileFunctionValue()
{
}

/*virtual*/ OpenFileFunctionValue::~OpenFileFunctionValue()
{
}

/*virtual*/ Powder::Value* OpenFileFunctionValue::Call(Powder::ListValue* argListValue, std::string& errorMsg)
{
	if (argListValue->Length() < 1)
	{
		errorMsg = Powder::FormatString("Open call requires at least 1 argument, got %d.", argListValue->Length());
		return nullptr;
	}

	GC::Reference<Value, true> filePathValueRef;
	argListValue->PopLeft(filePathValueRef);
	std::string filePath = filePathValueRef.Get()->ToString();

	FileValue* fileValue = new FileValue();

	std::ios_base::openmode openMode = std::fstream::in;
	GC::Reference<Value, true> openModeValueRef;
	if (argListValue->Length() > 0)
		argListValue->PopLeft(openModeValueRef);

	if (openModeValueRef.Get())
	{
		std::string openModeStr = openModeValueRef.Get()->ToString();
		if (openModeStr == "read")
			openMode = fileValue->fileStream.in;
		else if (openModeStr == "write")
			openMode = fileValue->fileStream.out | fileValue->fileStream.app;
		else
		{
			errorMsg = Powder::FormatString("Open mode \"%s\" not recognized.", openModeStr.c_str());
			return nullptr;
		}

		if (openModeStr == "write" && !std::filesystem::exists(filePath))
		{
			fileValue->fileStream.open(filePath, fileValue->fileStream.out);
			fileValue->fileStream.close();
		}
	}

	fileValue->fileStream.open(filePath, openMode);
	if (!fileValue->fileStream.is_open())
		return new Powder::UndefinedValue();

	return fileValue;
}