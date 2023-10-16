#include "MapValue.h"
#include "NullValue.h"
#include "NumberValue.h"
#include "ListValue.h"
#include "StringValue.h"
#include "BooleanValue.h"
#include "NullValue.h"

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
		return std::format("Map of size {}", this->valueMap.NumEntries());
	}

	/*virtual*/ std::string MapValue::GetTypeString() const
	{
		return "map";
	}

	/*virtual*/ std::string MapValue::GetSetKey() const
	{
		std::string key = "map:";

		MapValue* mapValue = const_cast<MapValue*>(this);
		for (HashMap<GC::Reference<Value, false>>::iterator iter = mapValue->valueMap.begin(); iter != mapValue->valueMap.end(); ++iter)
			key += (*iter).Get()->GetSetKey() + ",";

		return key;
	}

	/*virtual*/ Value* MapValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		switch (mathOp)
		{
			case MathInstruction::MathOp::SIZE:
			{
				return new NumberValue(this->valueMap.NumEntries());
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

		return new NullValue();
	}

	/*virtual*/ BooleanValue* MapValue::IsMember(const Value* value) const
	{
		std::string key = value->ToString();
		GC::Reference<Value, false>* valueRef = const_cast<MapValue*>(this)->valueMap.LookupPtr(key.c_str());
		return new BooleanValue(valueRef != nullptr && valueRef->Get() != nullptr);
	}

	void MapValue::SetField(const char* key, Value* dataValue)
	{
		this->valueMap.Insert(key, dataValue);
	}

	Value* MapValue::GetField(const char* key)
	{
		GC::Reference<Value, false>* dataValueRef = this->valueMap.LookupPtr(key);
		return dataValueRef ? dataValueRef->Get() : nullptr;
	}

	bool MapValue::DelField(const char* key, GC::Reference<Value, true>& valueRef)
	{
		GC::Reference<Value, false>* existingValueRef = this->valueMap.LookupPtr(key);
		if (existingValueRef)
		{
			valueRef.Set(existingValueRef->Get());
			this->valueMap.Remove(key);
			return true;
		}

		return false;
	}

	/*virtual*/ bool MapValue::SetField(Value* fieldValue, Value* dataValue, Error& error)
	{
		std::string key = fieldValue->ToString();
		this->SetField(key.c_str(), dataValue);
		return true;
	}

	/*virtual*/ Value* MapValue::GetField(Value* fieldValue, Error& error)
	{
		std::string key = fieldValue->ToString();
		return this->GetField(key.c_str());
	}

	/*virtual*/ bool MapValue::DelField(Value* fieldValue, GC::Reference<Value, true>& valueRef, Error& error)
	{
		std::string key = fieldValue->ToString();
		return this->DelField(key.c_str(), valueRef);
	}

	ListValue* MapValue::GenerateKeyListValue()
	{
		ListValue* listValue = new ListValue();
		this->valueMap.ForAllEntries([=](const char* key, GC::Reference<Value, false>& valueRef) -> bool {
			listValue->PushRight(new StringValue(key));
			return true;
		});
		return listValue;
	}

	/*virtual*/ CppFunctionValue* MapValue::MakeIterator(void)
	{
		return new MapValueIterator(this);
	}

	/*virtual*/ bool MapValue::IterationBegin(void*& userData)
	{
		auto iter = new HashMap<GC::Reference<Value, false>>::iterator();
		*iter = this->valueMap.begin();
		userData = iter;
		return true;
	}

	/*virtual*/ GC::Object* MapValue::IterationNext(void* userData)
	{
		auto iter = (HashMap<GC::Reference<Value, false>>::iterator*)userData;
		if (*iter == this->valueMap.end())
			return nullptr;
		GC::Object* object = &(**iter);
		++(*iter);
		return object;
	}

	/*virtual*/ void MapValue::IterationEnd(void* userData)
	{
		auto iter = (HashMap<GC::Reference<Value, false>>::iterator*)userData;
		delete iter;
	}

	MapValueIterator::MapValueIterator(MapValue* mapValue)
	{
		this->mapValueRef.Set(mapValue);
	}

	/*virtual*/ MapValueIterator::~MapValueIterator()
	{
	}

	/*virtual*/ bool MapValueIterator::Call(ListValue* argListValue, GC::Reference<Value, true>& returnValueRef, CppCallingContext& context, Error& error)
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
			this->mapIter = this->mapValueRef.Get()->GetValueMap().begin();
		else if (actionValue->GetString() == "next")
		{
			if (this->mapIter == this->mapValueRef.Get()->GetValueMap().end())
				returnValueRef.Set(new NullValue());
			else
			{
				returnValueRef.Set(new StringValue(this->mapIter.entry->key));
				++this->mapIter;
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