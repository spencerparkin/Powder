#include "StringValue.h"
#include "NullValue.h"
#include "BooleanValue.h"

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

	/*virtual*/ std::string StringValue::GetTypeString() const
	{
		return "string";
	}

	/*virtual*/ std::string StringValue::GetSetKey() const
	{
		return std::string("string:") + *this->str;
	}

	/*virtual*/ Value* StringValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		std::string strCast = value->ToString();
		
		switch (mathOp)
		{
			case MathInstruction::MathOp::ADD:
			{
				return new StringValue(*this->str + strCast);
			}
			case MathInstruction::MathOp::EQUAL:
			{
				return new BooleanValue(*this->str == strCast);
			}
			case MathInstruction::MathOp::NOT_EQUAL:
			{
				return new BooleanValue(*this->str != strCast);
			}
			case MathInstruction::MathOp::LESS_THAN:
			{
				return new BooleanValue(::strcmp(this->str->c_str(), strCast.c_str()) < 0);
			}
			case MathInstruction::MathOp::LESS_THAN_OR_EQUAL:
			{
				return new BooleanValue(::strcmp(this->str->c_str(), strCast.c_str()) <= 0);
			}
			case MathInstruction::MathOp::GREATER_THAN:
			{
				return new BooleanValue(::strcmp(this->str->c_str(), strCast.c_str()) > 0);
			}
			case MathInstruction::MathOp::GREATER_THAN_OR_EQUAL:
			{
				return new BooleanValue(::strcmp(this->str->c_str(), strCast.c_str()) >= 0);
			}
		}

		return new NullValue();
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
		{
			// TODO: Handle error here.
			number = 0.0;
		}
		return number;
	}
}