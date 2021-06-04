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

	/*virtual*/ uint32_t MathInstruction::Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine)
	{
		Value* rightValue = executor->GetCurrentScope()->PopValueFromEvaluationStackTop();
		Value* leftValue = executor->GetCurrentScope()->PopValueFromEvaluationStackTop();

		MathOp mathOp = MathOp(programBuffer[programBufferLocation + 1]);

		Value* result = leftValue->CombineWith(rightValue, mathOp, executor);
		if (!result)
			throw new RunTimeException(FormatString("Failed to combine operands in operation: 0x%04x", uint8_t(mathOp)));

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
				throw new CompileTimeException("Can't assemble math instruction if not given math operation code.");

			programBuffer[programBufferLocation + 1] = mathOpEntry->code;
		}

		programBufferLocation += 2L;
	}
}