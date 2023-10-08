#include "MathInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"
#include "ContainerValue.h"
#include "BooleanValue.h"
#include "Executor.h"
#include "Executable.h"
#include "ReferenceValue.h"
#include "VirtualMachine.h"

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
		else if (tokenText == "%")
			return MathOp::MODULUS;
		else if (tokenText == "==")
			return MathOp::EQUAL;
		else if (tokenText == "!=")
			return MathOp::NOT_EQUAL;
		else if (tokenText == "<")
			return MathOp::LESS_THAN;
		else if (tokenText == "<=")
			return MathOp::LESS_THAN_OR_EQUAL;
		else if (tokenText == ">")
			return MathOp::GREATER_THAN;
		else if (tokenText == ">=")
			return MathOp::GREATER_THAN_OR_EQUAL;
		else if (tokenText == "||")
			return MathOp::OR;
		else if (tokenText == "&&")
			return MathOp::AND;

		return MathOp::UNKNOWN;
	}

	/*static*/ MathInstruction::MathOp MathInstruction::TranslateUnaryLeftOperatorToken(const std::string& tokenText)
	{
		if (tokenText == "-")
			return MathInstruction::MathOp::NEGATE;
		else if (tokenText == "!")
			return MathInstruction::MathOp::NOT;
		else if (tokenText == "&")
			return MathInstruction::MathOp::REFERENCE;
		else if (tokenText == "@")
			return MathInstruction::MathOp::DEREFERENCE;

		return MathOp::UNKNOWN;
	}

	/*static*/ MathInstruction::MathOp MathInstruction::TranslateUnaryRightOperatorToken(const std::string& tokenText)
	{
		if (tokenText == "!")
			return MathInstruction::MathOp::FACTORIAL;

		return MathOp::UNKNOWN;
	}

	/*virtual*/ uint32_t MathInstruction::Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error)
	{
		GC::Reference<Value, true> resultRef;

		// It was decided that loading and storing (assignment), to and from scope,
		// would not be a math operation, and so it seems contradictory here to support
		// container setting and getting.  The reason I think there is no contradiction,
		// and that this choice keeps the system consistent, is because the math operation
		// only deals with concrete (rather than symbolic) values on the evaluation stack.
		// A list (or map) is itself a concrete value, as are the values used as fields,
		// or the values stored in the container at those fields.  Containers can store containers.
		const uint8_t* programBuffer = executable->byteCodeBuffer;
		uint8_t mathOp = programBuffer[programBufferLocation + 1];
		switch (mathOp)
		{
			case MathOp::GET_FIELD:
			{
				GC::Reference<Value, true> fieldValueRef, valueRef;
				if (!executor->PopValueFromEvaluationStackTop(fieldValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				ContainerValue* containerValue = dynamic_cast<ContainerValue*>(valueRef.Get());
				if (!containerValue)
				{
					error.Add("Get field math operation expected a container value on the evaluation stack.");
					return Executor::Result::RUNTIME_ERROR;
				}
				Value* value = containerValue->GetField(fieldValueRef.Get(), error);
				if (!value)
					return Executor::Result::RUNTIME_ERROR;
				resultRef.Set(value);
				break;
			}
			case MathOp::SET_FIELD:
			{
				if (!executor->PopValueFromEvaluationStackTop(resultRef, error))
					return Executor::Result::RUNTIME_ERROR;
				GC::Reference<Value, true> fieldValueRef, valueRef;
				if (!executor->PopValueFromEvaluationStackTop(fieldValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				ContainerValue* containerValue = dynamic_cast<ContainerValue*>(valueRef.Get());
				if (!containerValue)
				{
					error.Add("Set field math operation expected a container value on the evaluation stack.");
					return Executor::Result::RUNTIME_ERROR;
				}
				if (!containerValue->SetField(fieldValueRef.Get(), resultRef.Get(), error))
					return Executor::Result::RUNTIME_ERROR;
				if (virtualMachine->GetDebuggerTrap())
					virtualMachine->GetDebuggerTrap()->ValueChanged(containerValue);
				break;
			}
			case MathOp::DEL_FIELD:
			{
				GC::Reference<Value, true> fieldValueRef, valueRef;
				if (!executor->PopValueFromEvaluationStackTop(fieldValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				ContainerValue* containerValue = dynamic_cast<ContainerValue*>(valueRef.Get());
				if (!containerValue)
				{
					error.Add("Delete field math operation expected a container value on the evaluation stack.");
					return Executor::Result::RUNTIME_ERROR;
				}
				if (!containerValue->DelField(fieldValueRef.Get(), resultRef, error))
					return Executor::Result::RUNTIME_ERROR;
				if (virtualMachine->GetDebuggerTrap())
					virtualMachine->GetDebuggerTrap()->ValueChanged(containerValue);
				break;
			}
			case MathOp::CONTAINS:
			{
				GC::Reference<Value, true> valueRef;
				if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				ContainerValue* containerValue = dynamic_cast<ContainerValue*>(valueRef.Get());
				if (!containerValue)
				{
					error.Add("Membership math operation expected a container value on the evaluation stack.");
					return Executor::Result::RUNTIME_ERROR;
				}
				GC::Reference<Value, true> memberValueRef;
				if (!executor->PopValueFromEvaluationStackTop(memberValueRef, error))
					return Executor::Result::RUNTIME_ERROR;
				resultRef.Set(containerValue->IsMember(memberValueRef.Get()));
				break;
			}
			default:
			{
				bool unary = (mathOp & 0x80) != 0;
				mathOp &= ~0x80;
				if (unary)
				{
					GC::Reference<Value, true> valueRef;
					if (!executor->PopValueFromEvaluationStackTop(valueRef, error))
						return Executor::Result::RUNTIME_ERROR;
					switch (mathOp)
					{
						case MathOp::REFERENCE:
						{
							resultRef.Set(new ReferenceValue(valueRef.Get()));
							break;
						}
						case MathOp::DEREFERENCE:
						{
							ReferenceValue* refValue = dynamic_cast<ReferenceValue*>(valueRef.Get());
							if (!refValue)
							{
								error.Add("Tried to dereference a non-reference value.");
								return Executor::Result::RUNTIME_ERROR;
							}
							resultRef.Set(refValue->valueRef.Get());
							break;
						}
						default:
						{
							resultRef.Set(valueRef.Get()->CombineWith(nullptr, (MathOp)mathOp, executor));
							break;
						}
					}
				}
				else
				{
					GC::Reference<Value, true> rightValueRef, leftValueRef;
					if (!executor->PopValueFromEvaluationStackTop(rightValueRef, error))
						return Executor::Result::RUNTIME_ERROR;
					if (!executor->PopValueFromEvaluationStackTop(leftValueRef, error))
						return Executor::Result::RUNTIME_ERROR;
					resultRef.Set(leftValueRef.Get()->CombineWith(rightValueRef.Get(), (MathOp)mathOp, executor));
				}

				break;
			}
		}

		if (!resultRef.Get())
		{
			error.Add(std::format("Failed to combine operands in operation: {}", mathOp));
			return Executor::Result::RUNTIME_ERROR;
		}

		if (!executor->PushValueOntoEvaluationStackTop(resultRef.Get(), error))
			return Executor::Result::RUNTIME_ERROR;
		programBufferLocation += 2;
		return Executor::Result::CONTINUE;
	}

	/*virtual*/ bool MathInstruction::Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* mathOpEntry = this->assemblyData->configMap.LookupPtr("mathOp");
			if (!mathOpEntry)
			{
				error.Add(std::string(this->assemblyData->fileLocation) + "Can't assemble math instruction if not given math operation code.");
				return false;
			}

			uint8_t mathOp = mathOpEntry->code;
			if (mathOp == MathOp::UNKNOWN)
			{
				error.Add(std::string(this->assemblyData->fileLocation) + "Can't assemble math instruction with unknown math operation code.");
				return false;
			}

			switch (mathOp)
			{
				case MathOp::NEGATE:
				case MathOp::FACTORIAL:
				case MathOp::NOT:
				case MathOp::SIZE:
				case MathOp::REFERENCE:
				case MathOp::DEREFERENCE:
				{
					mathOp |= 0x80;
					break;
				}
			}

			uint8_t* programBuffer = executable->byteCodeBuffer;
			programBuffer[programBufferLocation + 1] = mathOp;
		}

		programBufferLocation += 2L;
		return true;
	}

	/*virtual*/ std::string MathInstruction::Print(void) const
	{
		std::string detail;
		detail += "math: ";
		const AssemblyData::Entry* mathOpEntry = this->assemblyData->configMap.LookupPtr("mathOp");
		detail += std::format("{}", (mathOpEntry ? mathOpEntry->code : -1));
		return detail;
	}
}