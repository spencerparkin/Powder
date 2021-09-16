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

	void MapValue::SetField(const char* key, Value* dataValue, bool decRefAfterSet)
	{
		Value* existingDataValue = this->valueMap.Lookup(key);
		if (existingDataValue)
		{
			this->valueMap.Remove(key);
			this->DisconnectFrom(existingDataValue);
		}

		if (dataValue)
		{
			this->valueMap.Insert(key, dataValue);
			this->ConnectTo(dataValue);
			if (decRefAfterSet)
				dataValue->DecRef();
		}
	}

	Value* MapValue::GetField(const char* key)
	{
		Value* dataValue = this->valueMap.Lookup(key);
		return dataValue;
	}

	Value* MapValue::DelField(const char* key, bool incRefBeforeDel)
	{
		Value* dataValue = this->valueMap.Lookup(key);
		if (dataValue)
		{
			if (incRefBeforeDel)
				dataValue->IncRef();
			this->valueMap.Remove(key);
			this->DisconnectFrom(dataValue);
		}
		return dataValue;
	}

	/*virtual*/ void MapValue::SetField(Value* fieldValue, Value* dataValue, bool decRefAfterSet)
	{
		std::string key = fieldValue->ToString();
		this->SetField(key.c_str(), dataValue, decRefAfterSet);
	}

	/*virtual*/ Value* MapValue::GetField(Value* fieldValue)
	{
		std::string key = fieldValue->ToString();
		return this->GetField(key.c_str());
	}

	/*virtual*/ Value* MapValue::DelField(Value* fieldValue, bool incRefBeforeDel)
	{
		std::string key = fieldValue->ToString();
		return this->DelField(key.c_str(), incRefBeforeDel);
	}

	ListValue* MapValue::GenerateKeyListValue()
	{
		ListValue* listValue = new ListValue();
		this->valueMap.ForAllEntries([=](const char* key, Value* value) -> bool {
			listValue->PushRight(new StringValue(key), true);
			return true;
		});
		return listValue;
	}

	/*virtual*/ CppFunctionValue* MapValue::MakeIterator(void)
	{
		return new MapValueIterator(this);
	}

	MapValueIterator::MapValueIterator(MapValue* mapValue) : mapValue(this)
	{
		this->mapValue.Set(mapValue);
	}

	/*virtual*/ MapValueIterator::~MapValueIterator()
	{
	}

	/*virtual*/ Value* MapValueIterator::Call(ListValue* argListValue, std::string& errorMsg)
	{
		if (argListValue->Length() != 1)
			return nullptr;

		const StringValue* actionValue = dynamic_cast<const StringValue*>((*argListValue)[0]);
		if (!actionValue)
			return nullptr;

		if (actionValue->GetString() == "reset")
			this->mapIter = (*mapValue).GetValueMap().begin();
		else if (actionValue->GetString() == "next")
		{
			Value* nextValue = nullptr;
			if (this->mapIter == mapValue.Get()->GetValueMap().end())
				nextValue = new UndefinedValue();
			else
			{
				nextValue = new StringValue(this->mapIter.entry->key);
				this->mapIter++;
			}
			return nextValue;
		}

		return nullptr;
	}
}