#include "NewComplexValueFunction.h"
#include "ListValue.h"
#include "ComplexValue.h"

NewComplexValueFunction::NewComplexValueFunction()
{
}

/*virtual*/ NewComplexValueFunction::~NewComplexValueFunction()
{
}

/*virtual*/ bool NewComplexValueFunction::Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::CppCallingContext& context, Powder::Error& error)
{
	if (argListValue->Length() != 2)
	{
		error.Add("New complex value function expected two arguments: a real part and an imaginary part.");
		return false;
	}

	GC::CriticalReference<Value> realValueRef, imagValueRef;
	argListValue->PopLeft(realValueRef, error);
	argListValue->PopLeft(imagValueRef, error);

	double realPart = realValueRef.Get()->AsNumber();
	double imagPart = imagValueRef.Get()->AsNumber();

	returnValueRef.Set(new ComplexValue(realPart, imagPart));
	return true;
}