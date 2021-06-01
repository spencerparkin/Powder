#include "ListValue.h"
#include "UndefinedValue.h"
#include "NumberValue.h"

namespace Powder
{
	ListValue::ListValue()
	{
		this->valueArray = new std::vector<Value*>();
	}

	/*virtual*/ ListValue::~ListValue()
	{
		delete this->valueArray;
	}

	/*virtual*/ Value* ListValue::Copy() const
	{
		//...
		return nullptr;
	}

	/*virtual*/ Value* ListValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		value = value->MathValue(executor);

		const ListValue* listValue = dynamic_cast<const ListValue*>(value);
		if (listValue)
		{
			switch (mathOp)
			{
				case MathInstruction::MathOp::ADD:
				{
					// TODO: Return new list value that is concatination of the list values.
					break;
				}
			}
		}

		const NumberValue* numberValue = dynamic_cast<const NumberValue*>(value);
		if (numberValue)
		{
			// TODO: Add math operation for indexing into the list.
		}

		return new UndefinedValue();
	}

	void ListValue::Clear()
	{
		//...
	}

	Value* ListValue::GetValueAt(uint64_t i)
	{
		if (this->valueArray->size() == 0)
			return nullptr;

		i = i % uint64_t(this->valueArray->size());
		return (*this->valueArray)[(unsigned int)i];
	}

	bool ListValue::SetValueAt(uint64_t i, Value* value)
	{
		if (valueArray->size() == 0)
			return false;

		i = i % uint64_t(this->valueArray->size());

		Value* existingValue = (*this->valueArray)[(unsigned int)i];
		if (existingValue)
			this->DisownObject(existingValue);

		(*this->valueArray)[(unsigned int)i] = value;

		if (value)
			this->OwnObject(value);

		return true;
	}
}