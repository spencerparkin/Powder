#include "Compiler.h"

namespace Powder
{
	Compiler::Compiler()
	{
	}

	/*virtual*/ Compiler::~Compiler()
	{
	}

	uint8_t* Compiler::CompileCode(const char* programCode, uint64_t& programBufferSize)
	{
		programBufferSize = 0L;

		//...
		
		// We first tokenize the given code into a list of tokens.
		// Then we build a syntax tree.
		// From here, things get a bit fuzzy, but to simplify matters,
		// I want subroutines to be the basic fundamental blocks of
		// executable code.  They will be compiled individually, and
		// then concatinated into one executable.  This means the blocks
		// will have to be re-locatable in memory, which means we'll
		// need to know how to patch the jump location.  I'm also going
		// to require a subroutine to be the entry-point of the program.
		// It will be the first subroutine at the top of the executable.
		// Not going to worry about local subroutines or lambdas.
		// Will probably never be able to support that.  It will be
		// amazing if I can just get it to work at all.
		//
		// One idea is to build a list of psuedo-instructions from
		// the syntax tree.  Each psuedo-instruction is just a
		// description of the final instruction.  Once we arrange
		// all the instructions to their final place, then we
		// can lay down the real instruction with a call to each
		// psuedo-instruction to render the instruction into the
		// final program buffer.

		return nullptr;
	}
}