#include "MathInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "ContainerValue.h"
#include "Exceptions.hpp"
#include "Executor.h"

namespace Powder
{
	MathInstruction::MathInstruction()
	{
	}

	/*virtual*/ MathInstruction::~MathInstruction()
	{
	}

	/*virtual*/ uint8_t MathInstruction::OpCode() const
	{
		return 0x05;
	}

	/*static*/ MathInstruction::MathOp MathInstruction::TranslateBinaryOperatorInfixToken(const std::string& tokenText)
	{
		if (tokenText == "+")
			return MathOp::ADD;
		else if (tokenText == "-")
			return MathOp::SUBTRACT;
		else if (tokenText == "*")
			return MathOp::MULTIPLY;
		else if (tokenText == "/")
			return MathOp::DIVIDE;
		else if (tokenText == "==")
			return MathOp::EQUAL;
		else if (tokenText == "<")
			return MathOp::LESS_THAN;
		else if (tokenText == "<=")
			return MathOp::LESS_THAN_OR_EQUAL;
		else if (tokenText == ">")
			return MathOp::GREATER_THAN;
		else if (tokenText == ">=")
			return MathOp::GREATER_THAN_OR_EQUAL;

		return MathOp::UNKNOWN;
	}

	/*static*/ MathInstruction::MathOp MathInstruction::TranslateUnaryLeftOperatorToken(const std::string& tokenText)
	{
		if (tokenText == "-")
			return MathInstruction::MathOp::NEGATE;
		else if (tokenText == "!")
			return MathInstruction::MathOp::NOT;

		return MathOp::UNKNOWN;
	}

	/*static*/ MathInstruction::MathOp MathInstruction::TranslateUnaryRightOperatorToken(const std::string& tokenText)
	{
		if (tokenText == "!")
			return MathInstruction::MathOp::FACTORIAL;

		return MathOp::UNKNOWN;
	}

	/*virtual*/ uint32_t MathInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		Value* result = nullptr;

		// It was decided that loading and storing (assignment), to and from scope,
		// would not be a math operation, and so it seems contradictory here to support
		// container setting and getting.  The reason I think there is no contradiction,
		// and that this choice keeps the system consistent, is because the math operation
		// only deals with concrete (rather than symbolic) values on the evaluation stack.
		// A list (or map) is itself a concrete value, as are the values used as fields,
		// or the values stored in the container at those fields.  Containers can store containers.
		uint8_t mathOp = programBuffer[programBufferLocation + 1];
		switch (mathOp)
		{
			case MathOp::GET_FIELD:
			{
				Value* fieldValue = executor->PopValueFromEvaluationStackTop();
				ContainerValue* containerValue = dynamic_cast<ContainerValue*>(executor->PopValueFromEvaluationStackTop());
				if (!containerValue)
					throw new RunTimeException("Get field math operation expected a container value on the evaluation stack.");
				result = containerValue->GetField(fieldValue);
				break;
			}
			case MathOp::SET_FIELD:
			{
				result = executor->PopValueFromEvaluationStackTop();
				Value* fieldValue = executor->PopValueFromEvaluationStackTop();
				ContainerValue* containerValue = dynamic_cast<ContainerValue*>(executor->PopValueFromEvaluationStackTop());
				if (!containerValue)
					throw new RunTimeException("Set field math operation expected a container value on the evaluation stack.");
				containerValue->SetField(fieldValue, result);
				break;
			}
			case MathOp::DEL_FIELD:
			{
				Value* fieldValue = executor->PopValueFromEvaluationStackTop();
				ContainerValue* containerValue = dynamic_cast<ContainerValue*>(executor->PopValueFromEvaluationStackTop());
				if (!containerValue)
					throw new RunTimeException("Delete field math operation expected a container value on the evaluation stack.");
				result = containerValue->DelField(fieldValue);
				break;
			}
			default:
			{
				bool unary = (mathOp & 0x80) != 0;
				mathOp &= ~0x80;
				if (unary)
				{
					Value* value = executor->PopValueFromEvaluationStackTop();
					result = value->CombineWith(nullptr, (MathOp)mathOp, executor);
				}
				else
				{
					Value* rightValue = executor->PopValueFromEvaluationStackTop();
					Value* leftValue = executor->PopValueFromEvaluationStackTop();
					result = leftValue->CombineWith(rightValue, (MathOp)mathOp, executor);
				}

				break;
			}
		}

		if (!result)
			throw new RunTimeException(FormatString("Failed to combine operands in operation: 0x%04x", mathOp));

		executor->PushValueOntoEvaluationStackTop(result);
		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void MathInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* mathOpEntry = this->assemblyData->configMap.LookupPtr("mathOp");
			if (!mathOpEntry)
				throw new CompileTimeException("Can't assemble math instruction if not given math operation code.");

			uint8_t mathOp = mathOpEntry->code;
			if (mathOp == MathOp::UNKNOWN)
				throw new CompileTimeException("Can't assemble math instruction with unknown math operation code.");

			switch (mathOp)
			{
				case MathOp::NEGATE:
				case MathOp::FACTORIAL:
				case MathOp::NOT:
				case MathOp::SIZE:
				{
					mathOp |= 0x80;
					break;
				}
			}

			programBuffer[programBufferLocation + 1] = mathOp;
		}

		programBufferLocation += 2L;
	}

#if defined POWDER_DEBUG
	/*virtual*/ std::string MathInstruction::Print(void) const
	{
		std::string detail;
		detail += "math: ";
		const AssemblyData::Entry* mathOpEntry = this->assemblyData->configMap.LookupPtr("mathOp");
		detail += FormatString("%04d", (mathOpEntry ? mathOpEntry->code : -1));
		return detail;
	}
#endif
}