#pragma once

#include "Value.h"

namespace Powder
{
	class POWDER_API ContainerValue : public Value
	{
	public:
		ContainerValue();
		virtual ~ContainerValue();

		virtual void SetField(Value* fieldValue, Value* dataValue) = 0;
		virtual Value* GetField(Value* fieldValue) = 0;
		virtual Value* DelField(Value* fieldValue) = 0;
	};
}