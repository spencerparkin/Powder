#pragma once

#include "Value.h"
#include <string>

namespace Powder
{
	// TODO: Derive from ContainerValue class.
	class POWDER_API StringValue : public Value
	{
	public:
		StringValue();
		StringValue(const std::string& str);
		virtual ~StringValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const override;
		virtual bool FromString(const std::string& str) override;
		virtual bool AsBoolean() const override;
		virtual double AsNumber() const override;
		virtual std::string GetTypeString() const override;
		virtual std::string GetSetKey() const override;

		void SetString(const std::string& str) { *this->str = str; }
		const std::string& GetString() const { return *this->str; }

	private:

		std::string* str;
	};
}