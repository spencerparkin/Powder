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

	SortedArrayQueue sortedArrayQueue;
	for (int i = 0; i < (signed)listValue->Length(); i++)
	{
		SortedArray* sortedArray = new SortedArray();
		sortedArray->push_back((*listValue)[i]);
		sortedArrayQueue.push_back(sortedArray);
	}

	SortedArrayQueue::iterator iter;
	while (sortedArrayQueue.size() > 1)
	{
		iter = sortedArrayQueue.begin();
		SortedArray* sortedArrayA = *iter;
		sortedArrayQueue.erase(iter);

		iter = sortedArrayQueue.begin();
		SortedArray* sortedArrayB = *iter;
		sortedArrayQueue.erase(iter);

		SortedArray* sortedArray = this->MergeSortedArrays(sortedArrayA, sortedArrayB, predicateValue, context, error);

		delete sortedArrayA;
		delete sortedArrayB;

		if (!sortedArray)
		{
			error.Add("Merge of sorted arrays failed.");

			while (sortedArrayQueue.size() > 0)
			{
				iter = sortedArrayQueue.begin();
				delete* iter;
				sortedArrayQueue.erase(iter);
			}

			return false;
		}

		sortedArrayQueue.push_back(sortedArray);
	}

	iter = sortedArrayQueue.begin();
	SortedArray* sortedArray = *iter;
	sortedArrayQueue.erase(iter);

	for (int i = 0; i < (signed)sortedArray->size(); i++)
		listValue->PushRight((*sortedArray)[i]);

	for (int i = 0; i < (signed)sortedArray->size(); i++)
	{
		GC::Reference<Value, true> valueRef;
		if (!listValue->PopLeft(valueRef, error))
			return false;
	}

	delete sortedArray;

	if (error.GetCount() > 0)
		return false;

	return true;
}

SortFunctionValue::SortedArray* SortFunctionValue::MergeSortedArrays(const SortedArray* sortedArrayA, const SortedArray* sortedArrayB, Powder::AddressValue* predicateValue, Powder::CppCallingContext& context, Powder::Error& error)
{
	SortedArray* sortedArray = new SortedArray();

	int i = 0;
	int j = 0;

	while (i < (signed)sortedArrayA->size() || j < (signed)sortedArrayB->size())
	{
		if (i < (signed)sortedArrayA->size() && j < (signed)sortedArrayB->size())
		{
			Value* valueA = (*sortedArray)[i];
			Value* valueB = (*sortedArray)[j];

			bool compareResult = false;

			if (!this->CompareValues(valueA, valueB, compareResult, predicateValue, context, error))
			{
				delete sortedArray;
				return nullptr;
			}

			if (compareResult)
			{
				sortedArray->push_back(valueA);
				i++;
			}
			else
			{
				sortedArray->push_back(valueB);
				j++;
			}
		}
		else if (i < (signed)sortedArrayA->size())
		{
			sortedArray->push_back((*sortedArrayA)[i++]);
		}
		else if (j < (signed)sortedArrayB->size())
		{
			sortedArray->push_back((*sortedArrayB)[j++]);
		}
	}

	return sortedArray;
}

bool SortFunctionValue::CompareValues(const Powder::Value* valueA, const Powder::Value* valueB, bool& compareResult, Powder::AddressValue* predicateValue, Powder::CppCallingContext& context, Powder::Error& error)
{
	// Note that I almost added some memoization to this subroutine, but assuming each
	// pointer in the list is unique, I don't think that we ever perform a redundant
	// comparison operation here, unless I'm mistaken.  Regardless, at some point, the
	// overhead in calling the predicate should be outweighed by trying to do a look-up
	// in a memoization map, for certain simple predicates.

	compareResult = false;

	GC::Reference<Powder::ListValue, true> predicateArgListValueRef(new Powder::ListValue());
	GC::Reference<Powder::Value, true> returnValueRef;

	predicateArgListValueRef.Get()->PushRight(const_cast<Powder::Value*>(valueA));
	predicateArgListValueRef.Get()->PushRight(const_cast<Powder::Value*>(valueB));

	if (!this->CallScriptFunction(predicateValue, predicateArgListValueRef, returnValueRef, context, error))
		return false;

	if (!returnValueRef.Get())
	{
		error.Add("Expected predicate to return a boolean value.");
		return false;
	}

	compareResult = returnValueRef.Get()->AsBoolean();

	return true;
}