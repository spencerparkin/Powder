#include "WriteLineFunctionValue.h"
#include "ListValue.h"
#include "FileValue.h"
#include "StringValue.h"

WriteLineFunctionValue::WriteLineFunctionValue()
{
}

/*virtual*/ WriteLineFunctionValue::~WriteLineFunctionValue()
{
}

/*virtual*/ bool WriteLineFunctionValue::Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::Error& error)
{
	if (argListValue->Length() != 2)
	{
		error.Add(std::format("Write-line call requires exactly 2 argument, got {}.", argListValue->Length()));
		return false;
	}

	GC::Reference<Value, true> fileValueRef;
	if (!argListValue->PopLeft(fileValueRef, error))
		return false;

	FileValue* fileValue = dynamic_cast<FileValue*>(fileValueRef.Get());
	if (!fileValue)
	{
		error.Add("Write-line call expected a file value as its first argument");
		return false;
	}

	GC::Reference<Value, true> lineValueRef;
	if (!argListValue->PopLeft(lineValueRef, error))
		return false;
	Powder::StringValue* lineValue = dynamic_cast<Powder::StringValue*>(lineValueRef.Get());
	if (!lineValue)
	{
		error.Add("Write-line call expected a string value as its second arguments.");
		return false;
	}

	if (!fileValue->fileStream.is_open())
	{
		error.Add("File given is not open.");
		return false;
	}

	fileValue->fileStream << lineValue->ToString() << std::endl;
	fileValue->fileStream.flush();
	return true;
}