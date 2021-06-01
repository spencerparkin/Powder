#include "VariableValue.h"
#include "Executor.h"
#include "Scope.h"
#include "UndefinedValue.h"
#include "Exceptions.hpp"

namespace Powder
{
	VariableValue::VariableValue()
	{
		this->identifier = new std::string;
	}

	VariableValue::VariableValue(const std::string& identifier)
	{
		this->identifier = new std::string;
		*this->identifier = identifier;
	}

	/*virtual*/ VariableValue::~VariableValue()
	{
		delete this->identifier;
	}

	/*virtual*/ Value* VariableValue::Copy() const
	{
		return new VariableValue(*this->identifier);
	}

	/*virtual*/ Value* VariableValue::CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const
	{
		if (mathOp == MathInstruction::MathOp::ASSIGN)
			executor->GetCurrentScope()->StoreValue(this->identifier->c_str(), const_cast<Value*>(value->MathValue(executor)));
		else
			this->MathValue(executor)->CombineWith(value, mathOp, executor);

		return new UndefinedValue();
	}

	/*virtual*/ const Value* VariableValue::MathValue(Executor* executor) const
	{
		Value* heldValue = executor->GetCurrentScope()->LookupValue(this->identifier->c_str(), false);
		if (!heldValue)
			throw new RunTimeException("Tried to use variable %s before it was ever assigned a value.");

		return heldValue;
	}
}