#include "ReadLineFunctionValue.h"
#include "ListValue.h"
#include "FileValue.h"
#include "StringValue.h"

ReadLineFunctionValue::ReadLineFunctionValue()
{
}

/*virtual*/ ReadLineFunctionValue::~ReadLineFunctionValue()
{
}

/*virtual*/ bool ReadLineFunctionValue::Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::CppCallingContext& context, Powder::Error& error)
{
	if (argListValue->Length() != 1)
	{
		error.Add(std::format("Read-line call requires exactly 1 argument, got {}.", argListValue->Length()));
		return false;
	}

	GC::Reference<Value, true> valueRef;
	if (!argListValue->PopLeft(valueRef, error))
		return false;

	FileValue* fileValue = dynamic_cast<FileValue*>(valueRef.Get());
	if (!fileValue)
	{
		error.Add("Read-line call expected a file value.");
		return false;
	}

	if (!fileValue->fileStream.is_open())
	{
		error.Add("File given is not open.");
		return false;
	}

	std::string line;
	if (!std::getline(fileValue->fileStream, line))
		line = "";

	Powder::StringValue* stringValue = new Powder::StringValue();
	stringValue->SetString(line);
	returnValueRef.Set(stringValue);
	return true;
}