#pragma once

#include "ContainerValue.h"
#include "CppFunctionValue.h"
#include "LinkedList.hpp"
#include "Reference.h"
#include <vector>

namespace Powder
{
	class POWDER_API ListValue : public ContainerValue
	{
		friend class ListValueIterator;

	public:
		ListValue();
		virtual ~ListValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const override;
		virtual bool SetField(Value* fieldValue, Value* dataValue, Error& error) override;
		virtual Value* GetField(Value* fieldValue, Error& error) override;
		virtual bool DelField(Value* fieldValue, GC::Reference<Value, true>& valueRef, Error& error) override;
		virtual BooleanValue* IsMember(const Value* value) const override;
		virtual CppFunctionValue* MakeIterator(void) override;
		virtual std::string GetTypeString() const override;
		virtual std::string GetSetKey() const override;

		void PushLeft(Value* value);
		bool PopLeft(GC::Reference<Value, true>& valueRef, Error& error);
		void PushRight(Value* value);
		bool PopRight(GC::Reference<Value, true>& valueRef, Error& error);

		unsigned int Length() const { return this->valueList.GetCount(); }
		Value* operator[](int i);
		const Value* operator[](int i) const;

		virtual bool IterationBegin(void*& userData) override;
		virtual Object* IterationNext(void* userData) override;
		virtual void IterationEnd(void* userData) override;

	private:
		LinkedList<GC::Reference<Value, false>> valueList;
		mutable std::vector<LinkedList<GC::Reference<Value, false>>::Node*>* valueListIndex;
		mutable bool valueListIndexValid;

		void RebuildIndexIfNeeded(void) const;
	};

	class POWDER_API ListValueIterator : public CppFunctionValue
	{
	public:
		ListValueIterator(ListValue* listValue);
		virtual ~ListValueIterator();

		virtual bool Call(ListValue* argListValue, GC::Reference<Value, true>& returnValueRef, CppCallingContext& context, Error& error) override;

		GC::Reference<ListValue, false> listValueRef;
		LinkedList<GC::Reference<Value, false>>::Node* listNode;
	};
}