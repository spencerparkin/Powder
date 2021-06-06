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
			UNKNOWN,

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
			NOT,

			// Binary comparison operations:
			EQUAL,
			LESS_THAN,
			LESS_THAN_OR_EQUAL,
			GREATER_THAN,
			GREATER_THAN_OR_EQUAL
		};

		static MathOp TranslateBinaryOperatorInfixToken(const std::string& tokenText);
		static MathOp TranslateUnaryLeftOperatorToken(const std::string& tokenText);
		static MathOp TranslateUnaryRightOperatorToken(const std::string& tokenText);

		virtual uint32_t Execute(const uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) override;
		virtual void Assemble(uint8_t* programBuffer, uint64_t programBufferSize, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const override;
	};
}