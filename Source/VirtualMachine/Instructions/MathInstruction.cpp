#include "MathInstruction.h"
#include "Assembler.h"
#include "Scope.h"
#include "Value.h"

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

	/*virtual*/ Executor::Result MathInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		Value* rightValue = nullptr;
		Value* leftValue = nullptr;

		executor->GetCurrentScope()->PopValueFromEvaluationStackTop(rightValue);
		executor->GetCurrentScope()->PopValueFromEvaluationStackTop(leftValue);

		MathOp mathOp = MathOp(programBuffer[programBufferLocation + 1]);

		Value* result = leftValue->CombineWith(rightValue, mathOp);
		if (!result)
		{
			// TODO: Throw exception.
		}

		executor->GetCurrentScope()->PushValueOntoEvaluationStackTop(result);

		programBufferLocation += 2;

		return Executor::Result::CONTINUE;
	}

	/*virtual*/ void MathInstruction::Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const
	{
		if (assemblyPass == AssemblyPass::RENDER)
		{
			const AssemblyData::Entry* mathOpEntry = this->assemblyData->configMap.LookupPtr("mathOp");
			if (!mathOpEntry)
			{
				// TODO: Throw exception.
			}

			if (mathOpEntry->string == "add")
				programBuffer[programBufferLocation + 1] = uint8_t(MathOp::ADD);
			else if (mathOpEntry->string == "subtract")
				programBuffer[programBufferLocation + 1] = uint8_t(MathOp::SUBTRACT);
			else if (mathOpEntry->string == "multiply")
				programBuffer[programBufferLocation + 1] = uint8_t(MathOp::MULTIPLY);
			else if (mathOpEntry->string == "divide")
				programBuffer[programBufferLocation + 1] = uint8_t(MathOp::DIVIDE);
			else
			{
				// TODO: Throw an exception.
			}
		}

		programBufferLocation += 2L;
	}
}