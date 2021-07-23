#include "MapValue.h"
#include "UndefinedValue.h"
#include "NumberValue.h"
#include "ListValue.h"
#include "Exceptions.hpp"
#include "StringFormat.h"
#include "StringValue.h"
#include "BooleanValue.h"

namespace Powder
{
	MapValue::MapValue()
	{
	}

	/*virtual*/ MapValue::~MapValue()
	{
	}

	/*virtual*/ Value* MapValue::Copy() const
	{
		return nullptr;
	}

	/*virtual*/ std::string MapValue::ToString() const
	{
		return FormatString("Map of size %d", this->valueMap.NumEntries());
	}

	/*virtual*/ Value* MapValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		switch (mathOp)
		{
			case MathInstruction::MathOp::SIZE:
			{
				return new NumberValue(this->valueMap.NumEntries());
			}
		}

		return new UndefinedValue();
	}

	/*virtual*/ BooleanValue* MapValue::IsMember(const Value* value) const
	{
		std::string key = value->ToString();
		Value* dataValue = const_cast<MapValue*>(this)->valueMap.Lookup(key.c_str());
		return new BooleanValue(dataValue != nullptr);
	}

	void MapValue::SetField(const char* key, Value* dataValue)
	{
		Value* existingDataValue = this->valueMap.Lookup(key);
		if (existingDataValue)
		{
			this->valueMap.Remove(key);
			this->DisownObject(existingDataValue);
		}

		if (dataValue)
		{
			this->valueMap.Insert(key, dataValue);
			this->OwnObject(dataValue);
		}
	}

	Value* MapValue::GetField(const char* key)
	{
		Value* dataValue = this->valueMap.Lookup(key);
		return dataValue;
	}

	/*virtual*/ void MapValue::SetField(Value* fieldValue, Value* dataValue)
	{
		std::string key = fieldValue->ToString();
		this->SetField(key.c_str(), dataValue);
	}

	/*virtual*/ Value* MapValue::GetField(Value* fieldValue)
	{
		std::string key = fieldValue->ToString();
		return this->GetField(key.c_str());
	}

	/*virtual*/ Value* MapValue::DelField(Value* fieldValue)
	{
		std::string key = fieldValue->ToString();
		Value* dataValue = this->valueMap.Lookup(key.c_str());
		if (dataValue)
		{
			this->valueMap.Remove(key.c_str());
			this->DisownObject(dataValue);
		}

		return dataValue;
	}

	ListValue* MapValue::GenerateKeyListValue()
	{
		ListValue* listValue = new ListValue();
		this->valueMap.ForAllEntries([=](const char* key, Value* value) -> bool {
			listValue->PushRight(new StringValue(key));
			return true;
		});
		return listValue;
	}

	/*virtual*/ CppFunctionValue* MapValue::MakeIterator(void)
	{
		return nullptr;
	}
}