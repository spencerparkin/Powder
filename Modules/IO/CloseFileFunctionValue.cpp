#include "CloseFileFunctionValue.h"
#include "ListValue.h"
#include "FileValue.h"
#include "BooleanValue.h"

CloseFileFunctionValue::CloseFileFunctionValue()
{
}

/*virtual*/ CloseFileFunctionValue::~CloseFileFunctionValue()
{
}

/*virtual*/ bool CloseFileFunctionValue::Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::VirtualMachine* virtualMachine, Powder::Error& error)
{
	if (argListValue->Length() != 1)
	{
		error.Add(std::format("Close call requires exactly 1 argument, got {}.", argListValue->Length()));
		return false;
	}

	GC::Reference<Value, true> valueRef;
	if (!argListValue->PopLeft(valueRef, error))
	{
		error.Add("Close call expected an argument.");
		return false;
	}

	FileValue* fileValue = dynamic_cast<FileValue*>(valueRef.Get());
	if (!fileValue)
	{
		error.Add("Close call expected a file value.");
		return false;
	}

	Powder::BooleanValue* resultValue = new Powder::BooleanValue();
	returnValueRef.Set(resultValue);

	if (!fileValue->fileStream.is_open())
		resultValue->SetBool(false);
	else
	{
		fileValue->fileStream.close();
		resultValue->SetBool(true);
	}

	return true;
}