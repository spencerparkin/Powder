#include "Executable.h"
#include "Exceptions.hpp"
#include <fstream>

namespace Powder
{
	Executable::Executable()
	{
		this->byteCodeBuffer = nullptr;
		this->byteCodeBufferSize = 0L;
	}

	/*virtual*/ Executable::~Executable()
	{
		this->Clear();
	}

	void Executable::Clear()
	{
		if (this->byteCodeBuffer)
		{
			delete[] this->byteCodeBuffer;
			this->byteCodeBuffer = nullptr;
			this->byteCodeBufferSize = 0L;
		}
	}

	void Executable::Save(const std::string& byteCodeFile) const
	{
		if (this->byteCodeBuffer && this->byteCodeBufferSize > 0)
		{
			std::fstream fileStream;
			fileStream.open(byteCodeFile, std::fstream::out | std::fstream::binary);
			if (!fileStream.is_open())
				throw new RunTimeException(FormatString("Failed to open file: %s", byteCodeFile.c_str()));

			fileStream.write((const char*)this->byteCodeBuffer, this->byteCodeBufferSize);
			fileStream.close();
		}
	}

	void Executable::Load(const std::string& byteCodeFile)
	{
		this->Clear();

		std::fstream fileStream;
		fileStream.open(byteCodeFile, std::fstream::in | std::fstream::binary | std::fstream::ate);
		if (!fileStream.is_open())
			throw new RunTimeException(FormatString("Failed to open file: %s", byteCodeFile.c_str()));

		this->byteCodeBufferSize = fileStream.tellg();
		if (this->byteCodeBufferSize == 0)
			throw new RunTimeException(FormatString("File (%s) is zero bytes in size.", byteCodeFile.c_str()));

		this->byteCodeBuffer = new uint8_t[(uint32_t)this->byteCodeBufferSize];
		fileStream.seekg(0, std::ios::beg);
		fileStream.read((char*)this->byteCodeBuffer, this->byteCodeBufferSize);
		fileStream.close();
	}
}