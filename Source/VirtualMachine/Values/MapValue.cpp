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
		return "";
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

	/*virtual*/ void MapValue::SetField(Value* fieldValue, Value* dataValue)
	{
		std::string key = fieldValue->ToString();

		Value* existingDataValue = this->valueMap.Lookup(key.c_str());
		if (existingDataValue)
		{
			this->valueMap.Remove(key.c_str());
			this->DisownObject(existingDataValue);
		}

		if (dataValue)
		{
			this->valueMap.Insert(key.c_str(), dataValue);
			this->OwnObject(dataValue);
		}
	}

	/*virtual*/ Value* MapValue::GetField(Value* fieldValue)
	{
		std::string key = fieldValue->ToString();
		Value* dataValue = this->valueMap.Lookup(key.c_str());
		return dataValue;
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
}