#pragma once

#include "ContainerValue.h"
#include "CppFunctionValue.h"
#include "LinkedList.hpp"
#include "GCSteward.hpp"
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
		virtual void SetField(Value* fieldValue, Value* dataValue) override;
		virtual Value* GetField(Value* fieldValue) override;
		virtual Value* DelField(Value* fieldValue) override;
		virtual BooleanValue* IsMember(const Value* value) const override;
		virtual CppFunctionValue* MakeIterator(void) override;

		void PushLeft(Value* value);
		Value* PopLeft();
		void PushRight(Value* value);
		Value* PopRight();

		unsigned int Length() const { return this->valueList.GetCount(); }
		Value* operator[](int i);
		const Value* operator[](int i) const;

	private:
		LinkedList<Value*> valueList;
		mutable std::vector<LinkedList<Value*>::Node*>* valueListIndex;
		mutable bool valueListIndexValid;

		void RebuildIndexIfNeeded(void) const;
	};

	class POWDER_API ListValueIterator : public CppFunctionValue
	{
	public:
		ListValueIterator(ListValue* listValue);
		virtual ~ListValueIterator();

		virtual Value* Call(ListValue* argListValue, std::string& errorMsg) override;

		GCSteward<ListValue> listValue;
		LinkedList<Value*>::Node* listNode;
	};
}