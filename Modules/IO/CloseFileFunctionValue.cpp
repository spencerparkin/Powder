#include "CloseFileFunctionValue.h"
#include "StringFormat.h"
#include "ListValue.h"
#include "FileValue.h"
#include "BooleanValue.h"

CloseFileFunctionValue::CloseFileFunctionValue()
{
}

/*virtual*/ CloseFileFunctionValue::~CloseFileFunctionValue()
{
}

/*virtual*/ Powder::Value* CloseFileFunctionValue::Call(Powder::ListValue* argListValue, std::string& errorMsg)
{
	if (argListValue->Length() != 1)
	{
		errorMsg = Powder::FormatString("Close call requires exactly 1 argument, got %d.", argListValue->Length());
		return nullptr;
	}

	GC::Reference<Value, true> valueRef;
	argListValue->PopLeft(valueRef);
	FileValue* fileValue = dynamic_cast<FileValue*>(valueRef.Get());
	if (!fileValue)
	{
		errorMsg = "Close call expected a file value.";
		return nullptr;
	}

	Powder::BooleanValue* resultValue = new Powder::BooleanValue();

	if (!fileValue->fileStream.is_open())
		resultValue->SetBool(false);
	else
	{
		fileValue->fileStream.close();
		resultValue->SetBool(true);
	}

	return resultValue;
}