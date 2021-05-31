#include "VariableValue.h"
#include "Executor.h"
#include "Scope.h"
#include "UndefinedValue.h"

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
		// When an identifier shows up in a math expression, the compiler should
		// most often produce instructions that load the value stored at that
		// identifier into the evaluation stack.  However, if an identifier
		// appears as the left-hand side of an assignment, then a push instruction
		// may be generated to push a variable value onto the evaluation stack.

		if(mathOp == MathInstruction::MathOp::ASSIGN)
			executor->GetCurrentScope()->StoreValue(this->identifier->c_str(), const_cast<Value*>(value));

		return new UndefinedValue();
	}
}