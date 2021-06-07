#include "MathInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
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

		uint8_t mathOp = programBuffer[programBufferLocation + 1];
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
				{
					mathOp |= 0x80;
					break;
				}
			}

			programBuffer[programBufferLocation + 1] = mathOp;
		}

		programBufferLocation += 2L;
	}
}