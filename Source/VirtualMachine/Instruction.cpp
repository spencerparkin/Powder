#include "Instruction.h"
#include "Assembler.h"

namespace Powder
{
	Instruction::Instruction()
	{
		this->assemblyData = nullptr;
	}

	/*virtual*/ Instruction::~Instruction()
	{
		delete this->assemblyData;
	}
}