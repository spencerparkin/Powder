#include "WriteLineFunctionValue.h"
#include "StringFormat.h"
#include "ListValue.h"
#include "FileValue.h"
#include "StringValue.h"

WriteLineFunctionValue::WriteLineFunctionValue()
{
}

/*virtual*/ WriteLineFunctionValue::~WriteLineFunctionValue()
{
}

/*virtual*/ Powder::Value* WriteLineFunctionValue::Call(Powder::ListValue* argListValue, std::string& errorMsg)
{
	if (argListValue->Length() != 2)
	{
		errorMsg = Powder::FormatString("Write-line call requires exactly 2 argument, got %d.", argListValue->Length());
		return nullptr;
	}

	FileValue* fileValue = dynamic_cast<FileValue*>(argListValue->PopLeft());
	if (!fileValue)
	{
		errorMsg = "Write-line call expected a file value as its first argument";
		return nullptr;
	}

	Powder::StringValue* lineValue = dynamic_cast<Powder::StringValue*>(argListValue->PopLeft());
	if (!lineValue)
	{
		errorMsg = "Write-line call expected a string value as its second arguments.";
		return nullptr;
	}

	if (!fileValue->fileStream.is_open())
	{
		errorMsg = "File given is not open.";
		return nullptr;
	}

	fileValue->fileStream << lineValue->ToString() << std::endl;
	fileValue->fileStream.flush();
	return nullptr;
}