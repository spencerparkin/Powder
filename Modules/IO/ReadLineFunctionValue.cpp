#include "ReadLineFunctionValue.h"
#include "StringFormat.h"
#include "ListValue.h"
#include "FileValue.h"
#include "StringValue.h"

ReadLineFunctionValue::ReadLineFunctionValue()
{
}

/*virtual*/ ReadLineFunctionValue::~ReadLineFunctionValue()
{
}

/*virtual*/ Powder::Value* ReadLineFunctionValue::Call(Powder::ListValue* argListValue, std::string& errorMsg)
{
	if (argListValue->Length() != 1)
	{
		errorMsg = Powder::FormatString("Read-line call requires exactly 1 argument, got %d.", argListValue->Length());
		return nullptr;
	}

	GC::Reference<Value, true> valueRef;
	argListValue->PopLeft(valueRef);
	FileValue* fileValue = dynamic_cast<FileValue*>(valueRef.Get());
	if (!fileValue)
	{
		errorMsg = "Read-line call expected a file value.";
		return nullptr;
	}

	if (!fileValue->fileStream.is_open())
	{
		errorMsg = "File given is not open.";
		return nullptr;
	}

	std::string line;
	if (!std::getline(fileValue->fileStream, line))
		line = "";

	Powder::StringValue* stringValue = new Powder::StringValue();
	stringValue->SetString(line);
	return stringValue;
}