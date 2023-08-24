#include "Executable.h"
#include "Exceptions.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace Powder
{
	Executable::Executable()
	{
		this->byteCodeBuffer = nullptr;
		this->byteCodeBufferSize = 0L;
		this->debugInfoDoc = nullptr;
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
			delete this->debugInfoDoc;
			this->debugInfoDoc = nullptr;
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

			if (this->debugInfoDoc)
			{
				std::string jsonString;
				if (!this->debugInfoDoc->PrintJson(jsonString))
					throw new RunTimeException("Failed to print debug info JSON string.");

				std::string debugInfoFile = byteCodeFile.substr(0, byteCodeFile.find_last_of('.')) + ".debuginfo";
				fileStream.open(debugInfoFile, std::fstream::out);
				if (!fileStream.is_open())
					throw new RunTimeException(FormatString("Failed to open file: %s", debugInfoFile.c_str()));

				fileStream.write(jsonString.c_str(), jsonString.length());
				fileStream.close();
			}
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

		std::string debugInfoFile = byteCodeFile.substr(0, byteCodeFile.find_last_of('.')) + ".debuginfo";
		if (std::filesystem::exists(debugInfoFile))
		{
			fileStream.open(debugInfoFile, std::fstream::in);
			if (!fileStream.is_open())
				throw new RunTimeException(FormatString("Failed to open file: %s", debugInfoFile.c_str()));

			std::stringstream stringStream;
			stringStream << fileStream.rdbuf();
			std::string jsonString = stringStream.str();

			std::string parseError;
			ParseParty::JsonValue* jsonValue = ParseParty::JsonValue::ParseJson(jsonString, parseError);
			if (!jsonValue)
				throw new RunTimeException(FormatString("Parse error in JSON file: %s\n\n%s", debugInfoFile.c_str(), parseError.c_str()));

			this->debugInfoDoc = dynamic_cast<ParseParty::JsonObject*>(jsonValue);
			if (!this->debugInfoDoc)
				throw new RunTimeException("Expected debug JSON info to be an object type.");
		}
	}
}