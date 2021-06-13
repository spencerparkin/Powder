#include "StringValue.h"
#include "UndefinedValue.h"
#include "Exceptions.hpp"

namespace Powder
{
	StringValue::StringValue()
	{
		this->str = new std::string();
	}

	StringValue::StringValue(const std::string& str)
	{
		this->str = new std::string();
		*this->str = str;
	}

	/*virtual*/ StringValue::~StringValue()
	{
		delete this->str;
	}

	/*virtual*/ Value* StringValue::Copy() const
	{
		return new StringValue(*this->str);
	}

	/*virtual*/ Value* StringValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		if (mathOp == MathInstruction::MathOp::ADD)
			return new StringValue(*this->str + value->ToString());

		return new UndefinedValue();
	}

	/*virtual*/ std::string StringValue::ToString() const
	{
		return *this->str;
	}

	/*virtual*/ bool StringValue::FromString(const std::string& str)
	{
		*this->str = str;
		return true;
	}

	/*virtual*/ bool StringValue::AsBoolean() const
	{
		return this->str->length() > 0;
	}

	/*virtual*/ double StringValue::AsNumber() const
	{
		char* endPtr = nullptr;
		double number = ::strtod(this->str->c_str(), &endPtr);
		if (endPtr == str->c_str())
			throw new RunTimeException("Failed to get string value as number.");
		return number;
	}
}