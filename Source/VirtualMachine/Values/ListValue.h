#pragma once

#include "ContainerValue.h"
#include "LinkedList.hpp"
#include <vector>

namespace Powder
{
	class POWDER_API ListValue : public ContainerValue
	{
	public:
		ListValue();
		virtual ~ListValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const override;
		virtual void SetField(Value* fieldValue, Value* dataValue) override;
		virtual Value* GetField(Value* fieldValue) override;
		virtual Value* DelField(Value* fieldValue) override;

		void PushLeft(Value* value);
		Value* PopLeft();
		void PushRight(Value* value);
		Value* PopRight();

	private:
		LinkedList<Value*> valueList;
		mutable std::vector<LinkedList<Value*>::Node*>* valueListIndex;
		mutable bool valueListIndexValid;

		void RebuildIndexIfNeeded(void) const;
	};
}