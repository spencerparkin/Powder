#include "ListValue.h"
#include "NullValue.h"
#include "NumberValue.h"
#include "BooleanValue.h"
#include "StringValue.h"

namespace Powder
{
	ListValue::ListValue()
	{
		this->valueListIndexValid = false;
		this->valueListIndex = new std::vector<LinkedList<GC::Reference<Value, false>>::Node*>;
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
		return std::format("List of length {}", this->Length());
	}

	/*virtual*/ std::string ListValue::GetTypeString() const
	{
		return "list";
	}

	/*virtual*/ std::string ListValue::GetSetKey() const
	{
		return std::format("list:{}", int(this));
	}

	/*virtual*/ Value* ListValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		switch (mathOp)
		{
			case MathInstruction::MathOp::SIZE:
			{
				return new NumberValue(this->Length());
			}
			case MathInstruction::MathOp::NOT_EQUAL:
			{
				if (dynamic_cast<const NullValue*>(value))
					return new BooleanValue(true);

				break;
			}
			case MathInstruction::MathOp::EQUAL:
			{
				if (dynamic_cast<const NullValue*>(value))
					return new BooleanValue(false);

				break;
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
					return newListValue;
				}
			}
		}

		return new NullValue();
	}

	/*virtual*/ BooleanValue* ListValue::IsMember(const Value* value) const
	{
		std::string valueStr = value->ToString();
		for (const LinkedList<GC::Reference<Value, false>>::Node* node = this->valueList.GetHead(); node; node = node->GetNext())
			if (node->value.Get()->ToString() == valueStr)
				return new BooleanValue(true);
		return new BooleanValue(false);
	}

	/*virtual*/ bool ListValue::SetField(Value* fieldValue, Value* dataValue, Error& error)
	{
		NumberValue* numberValue = dynamic_cast<NumberValue*>(fieldValue);
		if (!numberValue)
		{
			error.Add("Can't set field on list with something other than a number value.");
			return false;
		}

		int32_t i = int32_t(numberValue->AsNumber());
		if (i < 0 || i >= (signed)this->valueList.GetCount())
		{
			error.Add(std::format("Can't set element {} of list with {} elements.", i, this->valueList.GetCount()));
			return false;
		}

		this->RebuildIndexIfNeeded();
		(*this->valueListIndex)[i]->value.Set(dataValue);
		return true;
	}

	/*virtual*/ Value* ListValue::GetField(Value* fieldValue, Error& error)
	{
		NumberValue* numberValue = dynamic_cast<NumberValue*>(fieldValue);
		if (!numberValue)
		{
			error.Add("Can't get field on list with something other than a number value.");
			return nullptr;
		}

		int32_t i = int32_t(numberValue->AsNumber());
		if (i < 0 || i >= (signed)this->valueList.GetCount())
		{
			error.Add(std::format("Can't get element {} of list with {} elements.", i, this->valueList.GetCount()));
			return nullptr;
		}

		this->RebuildIndexIfNeeded();
		Value* dataValue = (*this->valueListIndex)[i]->value.Get();
		return dataValue;
	}

	/*virtual*/ bool ListValue::DelField(Value* fieldValue, GC::Reference<Value, true>& valueRef, Error& error)
	{
		NumberValue* numberValue = dynamic_cast<NumberValue*>(fieldValue);
		if (!numberValue)
		{
			error.Add("Can't delete field on list with something other than a number value.");
			return false;
		}

		int32_t i = int32_t(numberValue->AsNumber());
		if (i < 0 || i >= (signed)this->valueList.GetCount())
		{
			error.Add(std::format("Can't delete element {} of list with {} elements.", i, this->valueList.GetCount()));
			return false;
		}

		this->RebuildIndexIfNeeded();
		valueRef.Set((*this->valueListIndex)[i]->value.Get());
		this->valueList.Remove((*this->valueListIndex)[i]);
		this->valueListIndexValid = false;
		return true;
	}

	Value* ListValue::operator[](int i)
	{
		this->RebuildIndexIfNeeded();
		if (i >= 0 && i < (signed)this->valueList.GetCount())
			return (*this->valueListIndex)[i]->value.Get();
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
			for (LinkedList<GC::Reference<Value, false>>::Node* node = const_cast<LinkedList<GC::Reference<Value, false>>*>(&this->valueList)->GetHead(); node; node = node->GetNext())
				this->valueListIndex->push_back(node);
		}
	}

	void ListValue::PushLeft(Value* value)
	{
		this->valueList.AddHead(value);
		this->valueListIndexValid = false;
	}

	bool ListValue::PopLeft(GC::Reference<Value, true>& valueRef, Error& error)
	{
		if (this->valueList.GetCount() == 0)
		{
			error.Add("Tried to pop-left zero-size list.");
			return false;
		}

		valueRef.Set(this->valueList.GetHead()->value.Get());
		this->valueList.Remove(this->valueList.GetHead());
		this->valueListIndexValid = false;
		return true;
	}

	void ListValue::PushRight(Value* value)
	{
		this->valueList.AddTail(value);
		if (this->valueListIndexValid)
			this->valueListIndex->push_back(this->valueList.GetTail());
	}

	bool ListValue::PopRight(GC::Reference<Value, true>& valueRef, Error& error)
	{
		if (this->valueList.GetCount() == 0)
		{
			error.Add("Tried to pop-right zero-size list.");
			return false;
		}

		valueRef.Set(this->valueList.GetTail()->value.Get());
		this->valueList.Remove(this->valueList.GetTail());
		if (this->valueListIndexValid)
			this->valueListIndex->pop_back();
		return true;
	}

	/*virtual*/ CppFunctionValue* ListValue::MakeIterator(void)
	{
		return new ListValueIterator(this);
	}

	/*virtual*/ bool ListValue::IterationBegin(void*& userData)
	{
		auto nodePtr = new LinkedList<GC::Reference<Value, false>>::Node*;
		if (this->valueList.GetCount() == 0)
			*nodePtr = nullptr;
		else
			*nodePtr = this->valueList.GetHead();
		userData = nodePtr;
		return true;
	}

	/*virtual*/ GC::Object* ListValue::IterationNext(void* userData)
	{
		GC::Object* object = nullptr;

		auto nodePtr = (LinkedList<GC::Reference<Value, false>>::Node**)userData;
		if (*nodePtr)
		{
			object = &(*nodePtr)->value;
			*nodePtr = (*nodePtr)->GetNext();
		}

		return object;
	}

	/*virtual*/ void ListValue::IterationEnd(void* userData)
	{
		auto nodePtr = (LinkedList<GC::Reference<Value, false>>::Node**)userData;
		delete nodePtr;
	}

	ListValueIterator::ListValueIterator(ListValue* listValue)
	{
		this->listValueRef.Set(listValue);
		this->listNode = nullptr;
	}

	/*virtual*/ ListValueIterator::~ListValueIterator()
	{
	}

	/*virtual*/ bool ListValueIterator::Call(ListValue* argListValue, GC::Reference<Value, true>& returnValueRef, Error& error)
	{
		if (argListValue->Length() != 1)
		{
			error.Add("Iterator should be given an argument.");
			return false;
		}

		const StringValue* actionValue = dynamic_cast<const StringValue*>((*argListValue)[0]);
		if (!actionValue)
		{
			error.Add("Iterator argument should be a string.");
			return false;
		}

		if (actionValue->GetString() == "reset")
			this->listNode = this->listValueRef.Get()->valueList.GetHead();
		else if (actionValue->GetString() == "next")
		{
			if (!this->listNode)
				returnValueRef.Set(new NullValue());
			else
			{
				returnValueRef.Set(this->listNode->value.Get());
				this->listNode = this->listNode->GetNext();
			}
		}
		else
		{
			error.Add(std::format("Unrecognized action value: {}", actionValue->GetString().c_str()));
			return false;
		}

		return true;
	}
}