#include "ListValue.h"
#include "UndefinedValue.h"
#include "NumberValue.h"
#include "Exceptions.hpp"
#include "StringFormat.h"
#include "BooleanValue.h"
#include "StringValue.h"

namespace Powder
{
	ListValue::ListValue()
	{
		this->valueListIndexValid = false;
		this->valueListIndex = new std::vector<LinkedList<Value*>::Node*>;
	}

	/*virtual*/ ListValue::~ListValue()
	{
		delete this->valueListIndex;
	}

	/*virtual*/ Value* ListValue::Copy() const
	{
		return nullptr;
	}

	/*virtual*/ std::string ListValue::ToString() const
	{
		return FormatString("List of length %d", this->Length());
	}

	/*virtual*/ Value* ListValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		switch (mathOp)
		{
			case MathInstruction::MathOp::SIZE:
			{
				return new NumberValue(this->Length());
			}
		}

		const ListValue* listValue = dynamic_cast<const ListValue*>(value);
		if (listValue)
		{
			switch (mathOp)
			{
				case MathInstruction::MathOp::ADD:
				{
					ListValue* newListValue = new ListValue();
					newListValue->valueList.Append(this->valueList);
					newListValue->valueList.Append(listValue->valueList);
					for (LinkedList<Value*>::Node* node = newListValue->valueList.GetHead(); node; node = node->GetNext())
						newListValue->ConnectTo(node->value);
					return newListValue;
				}
			}
		}

		return new UndefinedValue();
	}

	/*virtual*/ BooleanValue* ListValue::IsMember(const Value* value) const
	{
		std::string valueStr = value->ToString();
		for (const LinkedList<Value*>::Node* node = this->valueList.GetHead(); node; node = node->GetNext())
			if (node->value->ToString() == valueStr)
				return new BooleanValue(true);
		return new BooleanValue(false);
	}

	/*virtual*/ void ListValue::SetField(Value* fieldValue, Value* dataValue)
	{
		NumberValue* numberValue = dynamic_cast<NumberValue*>(fieldValue);
		if (!numberValue)
			throw new RunTimeException("Can't set field on list with something other than a number value.");

		int32_t i = int32_t(numberValue->AsNumber());
		if (i < 0 || i >= (signed)this->valueList.GetCount())
			throw new RunTimeException(FormatString("Can't set element %d of list with %d elements.", i, this->valueList.GetCount()));

		this->RebuildIndexIfNeeded();
		Value* existingValue = (*this->valueListIndex)[i]->value;
		this->DisconnectFrom(existingValue);
		(*this->valueListIndex)[i]->value = dataValue;
		this->ConnectTo(dataValue);
	}

	/*virtual*/ Value* ListValue::GetField(Value* fieldValue)
	{
		NumberValue* numberValue = dynamic_cast<NumberValue*>(fieldValue);
		if (!numberValue)
			throw new RunTimeException("Can't get field on list with something other than a number value.");

		int32_t i = int32_t(numberValue->AsNumber());
		if (i < 0 || i >= (signed)this->valueList.GetCount())
			throw new RunTimeException(FormatString("Can't get element %d of list with %d elements.", i, this->valueList.GetCount()));

		this->RebuildIndexIfNeeded();
		Value* dataValue = (*this->valueListIndex)[i]->value;
		return dataValue;
	}

	/*virtual*/ Value* ListValue::DelField(Value* fieldValue)
	{
		NumberValue* numberValue = dynamic_cast<NumberValue*>(fieldValue);
		if (!numberValue)
			throw new RunTimeException("Can't delete field on list with something other than a number value.");

		int32_t i = int32_t(numberValue->AsNumber());
		if (i < 0 || i >= (signed)this->valueList.GetCount())
			throw new RunTimeException(FormatString("Can't delete element %d of list with %d elements.", i, this->valueList.GetCount()));

		this->RebuildIndexIfNeeded();
		Value* dataValue = (*this->valueListIndex)[i]->value;
		this->valueList.Remove((*this->valueListIndex)[i]);
		this->DisconnectFrom(dataValue);
		this->valueListIndexValid = false;
		return dataValue;
	}

	Value* ListValue::operator[](int i)
	{
		this->RebuildIndexIfNeeded();
		if (i >= 0 && i < (signed)this->valueList.GetCount())
			return (*this->valueListIndex)[i]->value;
		return nullptr;
	}

	const Value* ListValue::operator[](int i) const
	{
		return const_cast<ListValue*>(this)->operator[](i);
	}

	void ListValue::RebuildIndexIfNeeded(void) const
	{
		if (!this->valueListIndexValid)
		{
			this->valueListIndexValid = true;
			this->valueListIndex->clear();
			for (LinkedList<Value*>::Node* node = const_cast<LinkedList<Value*>*>(&this->valueList)->GetHead(); node; node = node->GetNext())
				this->valueListIndex->push_back(node);
		}
	}

	void ListValue::PushLeft(Value* value)
	{
		this->valueList.AddHead(value);
		this->valueListIndexValid = false;
		this->ConnectTo(value);
	}

	Value* ListValue::PopLeft()
	{
		if (this->valueList.GetCount() == 0)
			throw new RunTimeException("Tried to pop-left zero-size list.");
		Value* value = this->valueList.GetHead()->value;
		this->valueList.Remove(this->valueList.GetHead());
		this->valueListIndexValid = false;
		this->DisconnectFrom(value);
		return value;
	}

	void ListValue::PushRight(Value* value)
	{
		this->valueList.AddTail(value);
		if (this->valueListIndexValid)
			this->valueListIndex->push_back(this->valueList.GetTail());
		this->ConnectTo(value);
	}

	Value* ListValue::PopRight()
	{
		if (this->valueList.GetCount() == 0)
			throw new RunTimeException("Tried to pop-right zero-size list.");
		Value* value = this->valueList.GetTail()->value;
		this->valueList.Remove(this->valueList.GetTail());
		if (this->valueListIndexValid)
			this->valueListIndex->pop_back();
		this->DisconnectFrom(value);
		return value;
	}

	/*virtual*/ CppFunctionValue* ListValue::MakeIterator(void)
	{
		return new ListValueIterator(this);
	}

	ListValueIterator::ListValueIterator(ListValue* listValue) : listValue(this)
	{
		this->listValue.Set(listValue);
		this->listNode = nullptr;
	}

	/*virtual*/ ListValueIterator::~ListValueIterator()
	{
	}

	/*virtual*/ Value* ListValueIterator::Call(ListValue* argListValue, std::string& errorMsg)
	{
		if (argListValue->Length() != 1)
			return nullptr;

		const StringValue* actionValue = dynamic_cast<const StringValue*>((*argListValue)[0]);
		if (!actionValue)
			return nullptr;

		if (actionValue->GetString() == "reset")
			this->listNode = this->listValue.Get()->valueList.GetHead();
		else if (actionValue->GetString() == "next")
		{
			Value* nextValue = nullptr;
			if(!this->listNode)
				nextValue = new UndefinedValue();
			else
			{
				nextValue = this->listNode->value;
				this->listNode = this->listNode->GetNext();
			}
			return nextValue;
		}

		return nullptr;
	}
}