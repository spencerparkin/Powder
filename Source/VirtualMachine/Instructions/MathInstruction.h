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
			MODULUS,
			EXPONENTIATE,

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
			NOT_EQUAL,
			LESS_THAN,
			LESS_THAN_OR_EQUAL,
			GREATER_THAN,
			GREATER_THAN_OR_EQUAL,

			// List, map & set operations:
			GET_FIELD,			// Binary
			SET_FIELD,			// Ternary
			DEL_FIELD,			// Binary
			SIZE,				// Unary
			CONTAINS,			// Binary
		};

		static MathOp TranslateBinaryOperatorInfixToken(const std::string& tokenText);
		static MathOp TranslateUnaryLeftOperatorToken(const std::string& tokenText);
		static MathOp TranslateUnaryRightOperatorToken(const std::string& tokenText);

		virtual uint32_t Execute(GC::Reference<Executable, false>& executableRef, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine, Error& error) override;
		virtual bool Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass, Error& error) const override;
		virtual std::string Print(void) const override;
	};
}