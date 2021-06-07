#include "MapValue.h"
#include "UndefinedValue.h"
#include "Exceptions.hpp"
#include "StringFormat.h"

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
		//...
		return nullptr;
	}

	/*virtual*/ Value* MapValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		return new UndefinedValue();
	}

	/*virtual*/ void MapValue::SetField(Value* fieldValue, Value* dataValue)
	{
		// TODO: Don't forget to own value here.
	}

	/*virtual*/ Value* MapValue::GetField(Value* fieldValue)
	{
		return nullptr;
	}

	/*virtual*/ Value* MapValue::DelField(Value* fieldValue)
	{
		// TODO: Don't forget to disown value here.
		return nullptr;
	}
}