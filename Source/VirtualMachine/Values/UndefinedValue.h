#pragma once

#include "Value.h"

namespace Powder
{
	class UndefinedValue : public Value
	{
	public:
		UndefinedValue();
		virtual ~UndefinedValue();

		virtual Value* Copy() const;
	};
}