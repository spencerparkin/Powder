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

	/*virtual*/ Value* ListValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
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
		this->valueListIndexValid = false;
		return dataValue;
	}

	void ListValue::RebuildIndexIfNeeded(void)
	{
		if (!this->valueListIndexValid)
		{
			this->valueListIndexValid = true;
			this->valueListIndex->clear();
			for (LinkedList<Value*>::Node* node = this->valueList.GetHead(); node; node = node->GetNext())
				this->valueListIndex->push_back(node);
		}
	}
}