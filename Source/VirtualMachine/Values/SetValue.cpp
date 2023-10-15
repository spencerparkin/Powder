#include "SetValue.h"
#include "NumberValue.h"
#include "NullValue.h"
#include "BooleanValue.h"
#include "ListValue.h"
#include "StringValue.h"

using namespace Powder;

SetValue::SetValue()
{
	this->map = new Map();
}

/*virtual*/ SetValue::~SetValue()
{
	this->Clear();

	delete this->map;
}

void SetValue::Clear()
{
	for (const MapPair& pair : *this->map)
		delete pair.second;

	this->map->clear();
}

/*virtual*/ Value* SetValue::Copy() const
{
	SetValue* setValue = new SetValue();

	for (const MapPair& pair : *this->map)
		setValue->map->insert(MapPair(pair.first, new GC::Reference<Value, false>(pair.second->Get())));

	return setValue;
}

/*virtual*/ Value* SetValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
{
	Value* resultValue = nullptr;

	switch (mathOp)
	{
		case MathInstruction::MathOp::SIZE:
		{
			resultValue = new NumberValue(this->map->size());
			break;
		}
		case MathInstruction::MathOp::NOT_EQUAL:
		{
			const SetValue* setValue = dynamic_cast<const SetValue*>(value);
			if (setValue)
				resultValue = new BooleanValue(!this->IsEQualTo(setValue));
			else if (dynamic_cast<const NullValue*>(value))
				resultValue = new BooleanValue(true);
			break;
		}
		case MathInstruction::MathOp::EQUAL:
		{
			const SetValue* setValue = dynamic_cast<const SetValue*>(value);
			if (setValue)
				resultValue = new BooleanValue(this->IsEQualTo(setValue));
			else if (dynamic_cast<const NullValue*>(value))
				resultValue = new BooleanValue(false);
			break;
		}
		case MathInstruction::MathOp::ADD:
		{
			const SetValue* setValue = dynamic_cast<const SetValue*>(value);
			if (setValue)
				resultValue = this->CalculateUnionWith(setValue);
			break;
		}
		case MathInstruction::MathOp::SUBTRACT:
		{
			const SetValue* setValue = dynamic_cast<const SetValue*>(value);
			if (setValue)
				resultValue = this->CalculateIntersectionWith(setValue);
			break;
		}
		case MathInstruction::MathOp::EXPONENTIATE:
		{
			const SetValue* setValue = dynamic_cast<const SetValue*>(value);
			if (setValue)
				resultValue = this->CalculateDifferenceWith(setValue);
			break;
		}
	}

	return resultValue;
}

SetValue* SetValue::CalculateUnionWith(const SetValue* setValue) const
{
	SetValue* unionSet = (SetValue*)setValue->Copy();

	for (const MapPair& pair : *this->map)
		if(unionSet->map->find(pair.first) == unionSet->map->end())
			unionSet->map->insert(MapPair(pair.first, new GC::Reference<Value, false>(pair.second->Get())));

	return unionSet;
}

SetValue* SetValue::CalculateIntersectionWith(const SetValue* setValue) const
{
	SetValue* intersectionSet = new SetValue();

	for (const MapPair& pair : *this->map)
		if(setValue->map->find(pair.first) != setValue->map->end())
			intersectionSet->map->insert(MapPair(pair.first, new GC::Reference<Value, false>(pair.second->Get())));

	return intersectionSet;
}

SetValue* SetValue::CalculateDifferenceWith(const SetValue* setValue) const
{
	SetValue* differenceSet = new SetValue();

	for (const MapPair& pair : *this->map)
		if (setValue->map->find(pair.first) == setValue->map->end())
			differenceSet->map->insert(MapPair(pair.first, new GC::Reference<Value, false>(pair.second->Get())));

	return differenceSet;
}

bool SetValue::IsEQualTo(const SetValue* setValue) const
{
	for (const MapPair& pair : *this->map)
		if (setValue->map->find(pair.first) == setValue->map->end())
			return false;

	for (const MapPair& pair : *setValue->map)
		if (this->map->find(pair.first) == this->map->end())
			return false;

	return true;
}

/*virtual*/ std::string SetValue::ToString() const
{
	return std::format("Set of size {}", this->map->size());
}

/*virtual*/ std::string SetValue::GetTypeString() const
{
	return "set";
}

/*virtual*/ std::string SetValue::GetSetKey() const
{
	std::string key = "set:";

	for (const MapPair& pair : *this->map)
		key += pair.first + ",";

	return key;
}

/*virtual*/ BooleanValue* SetValue::IsMember(const Value* value) const
{
	return new BooleanValue(this->map->find(value->GetSetKey()) != this->map->end());
}

/*virtual*/ bool SetValue::AddMember(Value* value, Error& error)
{
	if (this->map->find(value->GetSetKey()) == this->map->end())
	{
		this->map->insert(MapPair(value->GetSetKey(), new GC::Reference<Value, false>(value)));
		return true;
	}

	return false;
}

/*virtual*/ bool SetValue::RemoveMember(Value* value, Error& error)
{
	Map::iterator iter = this->map->find(value->GetSetKey());
	if (iter != this->map->end())
	{
		delete iter->second;
		this->map->erase(iter);
		return true;
	}

	return false;
}

/*virtual*/ CppFunctionValue* SetValue::MakeIterator(void)
{
	return new SetValueIterator(this);
}

/*virtual*/ bool SetValue::IterationBegin(void*& userData)
{
	auto iter = new Map::iterator();
	*iter = this->map->begin();
	userData = iter;
	return true;
}

/*virtual*/ GC::Object* SetValue::IterationNext(void* userData)
{
	auto iter = (Map::iterator*)userData;
	if (*iter == this->map->end())
		return nullptr;
	GC::Object* object = (**iter).second;
	++(*iter);
	return object;
}

/*virtual*/ void SetValue::IterationEnd(void* userData)
{
	auto iter = (Map::iterator*)userData;
	delete iter;
}

SetValueIterator::SetValueIterator(SetValue* setValue)
{
	this->setValueRef.Set(setValue);
	this->mapIter = new SetValue::Map::iterator();
}

/*virtual*/ SetValueIterator::~SetValueIterator()
{
	delete this->mapIter;
}

/*virtual*/ bool SetValueIterator::Call(ListValue* argListValue, GC::Reference<Value, true>& returnValueRef, Error& error)
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
		*this->mapIter = this->setValueRef.Get()->map->begin();
	else if (actionValue->GetString() == "next")
	{
		if (*this->mapIter == this->setValueRef.Get()->map->end())
			returnValueRef.Set(new NullValue());
		else
		{
			returnValueRef.Set((*this->mapIter)->second->Get());
			++(*this->mapIter);
		}
	}
	else
	{
		error.Add(std::format("Unrecognized action value: {}", actionValue->GetString().c_str()));
		return false;
	}

	return true;
}