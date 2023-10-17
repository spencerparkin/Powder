#pragma once

#include "CppFunctionValue.h"
#include "AddressValue.h"

class SortFunctionValue : public Powder::CppFunctionValue
{
public:
	SortFunctionValue();
	virtual ~SortFunctionValue();

	virtual bool Call(Powder::ListValue* argListValue, GC::Reference<Powder::Value, true>& returnValueRef, Powder::CppCallingContext& context, Powder::Error& error) override;

private:

	typedef std::vector<Powder::Value*> SortedArray;
	typedef std::list<SortedArray*> SortedArrayQueue;

	SortedArray* MergeSortedArrays(const SortedArray* sortedArrayA, const SortedArray* sortedArrayB, Powder::AddressValue* predicateValue, Powder::CppCallingContext& context, Powder::Error& error);
	bool CompareValues(const Powder::Value* valueA, const Powder::Value* valueB, bool& compareResult, Powder::AddressValue* predicateValue, Powder::CppCallingContext& context, Powder::Error& error);
};