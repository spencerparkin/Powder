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

	std::string Instruction::ExtractEmbeddedString(const uint8_t* programBuffer, uint64_t programBufferLocation)
	{
		std::string str;
		while (programBuffer[programBufferLocation] != '\0')
			str += programBuffer[programBufferLocation++];
		return str;
	}
}