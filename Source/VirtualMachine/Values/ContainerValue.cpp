#include "ContainerValue.h"

namespace Powder
{
	ContainerValue::ContainerValue()
	{
	}

	/*virtual*/ ContainerValue::~ContainerValue()
	{
	}

	/*virtual*/ bool ContainerValue::SetField(Value* fieldValue, Value* dataValue, Error& error)
	{
		error.Add("No supported.");
		return false;
	}

	/*virtual*/ Value* ContainerValue::GetField(Value* fieldValue, Error& error)
	{
		error.Add("No supported.");
		return nullptr;
	}

	/*virtual*/ bool ContainerValue::DelField(Value* fieldValue, GC::Reference<Value, true>& valueRef, Error& error)
	{
		error.Add("No supported.");
		return false;
	}

	/*virtual*/ bool ContainerValue::AddMember(Value* value, Error& error)
	{
		error.Add("Not supported!");
		return false;
	}

	/*virtual*/ bool ContainerValue::RemoveMember(Value* value, Error& error)
	{
		error.Add("Not supported!");
		return false;
	}

	/*virtual*/ Value* ContainerValue::AnyMember(Error& error)
	{
		error.Add("Not supported!");
		return nullptr;
	}

	// TODO: Oops!  What if you try to mutate a container while iterating over it?  Couldn't this cause a crash?
	//       Should all containers own a lock bit and raise a run-time error if you try to mutate them during iteration?
}