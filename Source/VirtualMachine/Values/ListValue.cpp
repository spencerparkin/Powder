#include "ListValue.h"

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

	/*virtual*/ Value* ListValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp) const
	{
		return nullptr;
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