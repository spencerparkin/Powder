#include "SortFunctionValue.h"
#include "ListValue.h"
#include "AddressValue.h"
#include "Executable.h"

SortFunctionValue::SortFunctionValue()
{
}

/*virtual*/ SortFunctionValue::~SortFunctionValue()
{
}

/*virtual*/ bool SortFunctionValue::Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::CppCallingContext& context, Powder::Error& error)
{
	if (argListValue->Length() != 2)
	{
		error.Add("Sort function expected exactly two arguments: a list value, then a predicate function value.");
		return false;
	}

	GC::Reference<Powder::Value, true> listValueRef;
	if (!argListValue->PopLeft(listValueRef, error))
		return false;

	GC::Reference<Powder::Value, true> predicateValueRef;
	if (!argListValue->PopLeft(predicateValueRef, error))
		return false;

	Powder::ListValue* listValue = dynamic_cast<Powder::ListValue*>(listValueRef.Get());
	if (!listValue)
	{
		error.Add("Sort function expected first argument to be a list value.");
		return false;
	}

	Powder::AddressValue* predicateValue = dynamic_cast<Powder::AddressValue*>(predicateValueRef.Get());
	if (!predicateValue)
	{
		error.Add("Sort function expected second argument to be a function value.");
		return false;
	}

	// TODO: I'm not happy about our inability to do proper error-handling here.
	listValue->SortWithPredicate([=, &listValue, &predicateValue, &context, &error](const Powder::LinkedList<GC::Reference<Value, false>>::Node* nodeA, const Powder::LinkedList<GC::Reference<Value, false>>::Node* nodeB) -> bool {
		GC::Reference<Value, true> returnValueRef;
		GC::Reference<Powder::ListValue, true> predicateArgListValueRef(new Powder::ListValue());
		predicateArgListValueRef.Get()->PushRight(const_cast<Powder::Value*>(nodeA->value.Get()));
		predicateArgListValueRef.Get()->PushRight(const_cast<Powder::Value*>(nodeB->value.Get()));
		this->CallScriptFunction(predicateValue, predicateArgListValueRef, returnValueRef, context, error);
		return returnValueRef.Get()->AsBoolean();
	});

	if (error.GetCount() > 0)
		return false;

	return true;
}