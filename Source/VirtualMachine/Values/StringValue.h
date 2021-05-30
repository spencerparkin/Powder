#pragma once

#include "Value.h"
#include <string>

namespace Powder
{
	class POWDER_API StringValue : public Value
	{
	public:
		StringValue();
		StringValue(const std::string& str);
		virtual ~StringValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp) const override;

	private:

		std::string* str;
	};
}