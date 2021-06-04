#pragma once

#include "Instruction.h"

namespace Powder
{
	class POWDER_API MathInstruction : public Instruction
	{
	public:
		MathInstruction();
		virtual ~MathInstruction();

		virtual uint8_t OpCode() const override;

		enum MathOp
		{
			// Binary arithmetic operations:
			ADD,
			SUBTRACT,
			MULTIPLY,
			DIVIDE,

			// Unary arithmetic operations:
			NEGATE,
			FACTORIAL,

			// Binary logic operations:
			AND,
			OR,
			XOR,

			// Unary logic operations:
			NOT
		};

		virtual uint32_t Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;
	};
}