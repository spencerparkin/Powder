#include "ListValue.h"
#include "UndefinedValue.h"
#include "NumberValue.h"
#include "Exceptions.hpp"
#include "StringFormat.h"

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
		std::string listStr = "[";
		this->RebuildIndexIfNeeded();
		for (int i = 0; i < (signed)this->valueListIndex->size(); i++)
		{
			listStr += (*this->valueListIndex)[i]->value->ToString();
			if (i + 1 < (signed)this->valueListIndex->size())
				listStr += ", ";
		}
		listStr += "]";
		return listStr;
	}

	/*virtual*/ Value* ListValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		switch (mathOp)
		{
			case MathInstruction::MathOp::SIZE:
			{
				return new NumberValue(this->valueList.GetCount());
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
						newListValue->OwnObject(node->value);
					return newListValue;
				}
			}
		}

		return new UndefinedValue();
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
		this->DisownObject(existingValue);
		(*this->valueListIndex)[i]->value = dataValue;
		this->OwnObject(dataValue);
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
		this->DisownObject(dataValue);
		this->valueListIndexValid = false;
		return dataValue;
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
		this->OwnObject(value);
	}

	Value* ListValue::PopLeft()
	{
		if (this->valueList.GetCount() == 0)
			throw new RunTimeException("Tried to pop-left zero-size list.");
		Value* value = this->valueList.GetHead()->value;
		this->valueList.Remove(this->valueList.GetHead());
		this->valueListIndexValid = false;
		this->DisownObject(value);
		return value;
	}

	void ListValue::PushRight(Value* value)
	{
		this->valueList.AddTail(value);
		if (this->valueListIndexValid)
			this->valueListIndex->push_back(this->valueList.GetTail());
		this->OwnObject(value);
	}

	Value* ListValue::PopRight()
	{
		if (this->valueList.GetCount() == 0)
			throw new RunTimeException("Tried to pop-right zero-size list.");
		Value* value = this->valueList.GetTail()->value;
		this->valueList.Remove(this->valueList.GetTail());
		if (this->valueListIndexValid)
			this->valueListIndex->pop_back();
		this->DisownObject(value);
		return value;
	}
}