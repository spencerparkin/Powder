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
	for (const std::pair<Value*, GC::Reference<Value, false>*>& pair : *this->map)
		delete pair.second;

	this->map->clear();
}

/*virtual*/ Value* SetValue::Copy() const
{
	SetValue* setValue = new SetValue();

	for (const std::pair<Value*, GC::Reference<Value, false>*>& pair : *this->map)
		setValue->map->insert(std::pair<Value*, GC::Reference<Value, false>*>(pair.first, new GC::Reference<Value, false>(pair.second->Get())));

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

	for (const std::pair<Value*, GC::Reference<Value, false>*>& pair : *this->map)
		if(unionSet->map->find(pair.first) == unionSet->map->end())
			unionSet->map->insert(std::pair<Value*, GC::Reference<Value, false>*>(pair.first, new GC::Reference<Value, false>(pair.second->Get())));

	return unionSet;
}

SetValue* SetValue::CalculateIntersectionWith(const SetValue* setValue) const
{
	SetValue* intersectionSet = new SetValue();

	for (const std::pair<Value*, GC::Reference<Value, false>*>& pair : *this->map)
		if(setValue->map->find(pair.first) != setValue->map->end())
			intersectionSet->map->insert(std::pair<Value*, GC::Reference<Value, false>*>(pair.first, new GC::Reference<Value, false>(pair.second->Get())));

	return intersectionSet;
}

SetValue* SetValue::CalculateDifferenceWith(const SetValue* setValue) const
{
	SetValue* differenceSet = new SetValue();

	for (const std::pair<Value*, GC::Reference<Value, false>*>& pair : *this->map)
		if (setValue->map->find(pair.first) == setValue->map->end())
			differenceSet->map->insert(std::pair<Value*, GC::Reference<Value, false>*>(pair.first, new GC::Reference<Value, false>(pair.second->Get())));

	return differenceSet;
}

bool SetValue::IsEQualTo(const SetValue* setValue) const
{
	for (const std::pair<Value*, GC::Reference<Value, false>*>& pair : *this->map)
		if (setValue->map->find(pair.first) == setValue->map->end())
			return false;

	for (const std::pair<Value*, GC::Reference<Value, false>*>& pair : *setValue->map)
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

/*virtual*/ bool SetValue::SetField(Value* fieldValue, Value* dataValue, Error& error)
{
	if (fieldValue)
	{
		error.Add("Field value not used with sets.");
		return false;
	}

	if (this->map->find(dataValue) == this->map->end())
		this->map->insert(std::pair<Value*, GC::Reference<Value, false>*>(dataValue, new GC::Reference<Value, false>(dataValue)));

	return true;
}

/*virtual*/ Value* SetValue::GetField(Value* fieldValue, Error& error)
{
	error.Add("No supported.");
	return nullptr;
}

/*virtual*/ bool SetValue::DelField(Value* fieldValue, GC::Reference<Value, true>& valueRef, Error& error)
{
	if (fieldValue)
	{
		error.Add("Field value not used with sets.");
		return false;
	}

	if (this->map->size() == 0)
	{
		error.Add("Can't remove element from empty set.");
		return false;
	}

	Map::iterator iter = this->map->begin();
	valueRef.Set(iter->first);
	this->map->erase(iter);
	return true;
}

/*virtual*/ BooleanValue* SetValue::IsMember(const Value* value) const
{
	return new BooleanValue(this->map->find(const_cast<Value*>(value)) != this->map->end());
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
			returnValueRef.Set((*this->mapIter)->first);
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