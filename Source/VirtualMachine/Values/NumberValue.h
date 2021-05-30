#pragma once

#include "Value.h"

namespace Powder
{
	class NumberValue : public Value
	{
	public:
		NumberValue();
		NumberValue(double number);
		virtual ~NumberValue();

		virtual Value* Copy() const;

		double number;
	};
}