#include "OpenFileFunctionValue.h"
#include "ListValue.h"
#include "FileValue.h"
#include "NullValue.h"
#include "Error.h"
#include <filesystem>

OpenFileFunctionValue::OpenFileFunctionValue()
{
}

/*virtual*/ OpenFileFunctionValue::~OpenFileFunctionValue()
{
}

/*virtual*/ bool OpenFileFunctionValue::Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::CppCallingContext& context, Powder::Error& error)
{
	if (argListValue->Length() < 1)
	{
		error.Add(std::format("Open call requires at least 1 argument, got {}.", argListValue->Length()));
		return false;
	}

	GC::CriticalReference<Value> filePathValueRef;
	if (!argListValue->PopLeft(filePathValueRef, error))
		return false;

	std::string filePath = filePathValueRef.Get()->ToString();

	FileValue* fileValue = new FileValue();
	returnValueRef.Set(fileValue);

	std::ios_base::openmode openMode = std::fstream::in;
	GC::CriticalReference<Value> openModeValueRef;
	if (argListValue->Length() > 0)
		if (!argListValue->PopLeft(openModeValueRef, error))
			return false;

	if (openModeValueRef.Get())
	{
		std::string openModeStr = openModeValueRef.Get()->ToString();
		if (openModeStr == "read")
			openMode = fileValue->fileStream.in;
		else if (openModeStr == "write")
			openMode = fileValue->fileStream.out | fileValue->fileStream.app;
		else
		{
			error.Add(std::format("Open mode \"{}\" not recognized.", openModeStr.c_str()));
			return false;
		}

		if (openModeStr == "write" && !std::filesystem::exists(filePath))
		{
			fileValue->fileStream.open(filePath, fileValue->fileStream.out);
			fileValue->fileStream.close();
		}
	}

	fileValue->fileStream.open(filePath, openMode);
	if (!fileValue->fileStream.is_open())
		returnValueRef.Set(new Powder::NullValue());

	return true;
}