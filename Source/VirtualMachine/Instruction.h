#pragma once

#include "Defines.h"
#include "Lexer.h"
#include <cstdint>
#include <string>

namespace Powder
{
	class VirtualMachine;
	class Executor;
	class Executable;
	struct AssemblyData;

	// Derivatives of this class are responsible for both the decoding
	// and encoding of a Powder VM instructions.
	class POWDER_API Instruction
	{
	public:
		Instruction();
		virtual ~Instruction();

		virtual uint8_t OpCode() const = 0;

		// Read and execute this instruction from the given executable buffer.
		// Note that for faster execution, implimentations of this virtual
		// method need not bounds-check their access to the given program
		// buffer.  An out-of-bounds error here means there is a bug in the compiler.
		// The override should always advance or otherwise update the given program
		// buffer location, and may change the given exectuable pointer.
		virtual uint32_t Execute(const Executable*& executable, uint64_t& programBufferLocation, Executor* executor, VirtualMachine* virtualMachine) = 0;

		enum AssemblyPass
		{
			// In this pass, nothing is written to the program buffer.
			// Rather, it is expected that the given program location
			// just be advanced by the size of the instruction.  In
			// addition to helping us calculate the overall executable
			// buffer size, this also allows us to locate all the instructions
			// in the buffer before they're actually written to the buffer.
			CALC_EXTENT,

			// In this pass, the instruction gets written to the given
			// program buffer.  Since all instruction locations were
			// determined in the first pass, this allows the instruction
			// to resolve pointer references it has to other instructions,
			// if any, as location offsets into the program buffer.
			RENDER
		};

		// Format and write this instruction into the given buffer.
		// Note that overrides need not write their op-code into
		// the buffer; the assembler does that for you.
		virtual void Assemble(Executable* executable, uint64_t& programBufferLocation, AssemblyPass assemblyPass) const = 0;

#if defined POWDER_DEBUG
		// This lets us print a disassembled version of the program.
		virtual std::string Print(void) const = 0;
#endif

		// This pointer is setup by the compiler and used during
		// assembly to know how to configure and link the instruction.
		AssemblyData* assemblyData;

		template<typename T>
		static T* CreateForAssembly(const ParseParty::Lexer::FileLocation& fileLocation)
		{
			T* instruction = new T;
			instruction->assemblyData = new AssemblyData;
			instruction->assemblyData->fileLocation = fileLocation;
			return instruction;
		}

	protected:

		std::string ExtractEmbeddedString(const uint8_t* programBuffer, uint64_t programBufferLocation);
	};
}