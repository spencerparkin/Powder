#include "Executable.h"
#include "Error.h"
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

	bool Executable::Save(const std::string& byteCodeFile, Error& error) const
	{
		if (this->byteCodeBuffer && this->byteCodeBufferSize > 0)
		{
			std::fstream fileStream;
			fileStream.open(byteCodeFile, std::fstream::out | std::fstream::binary);
			if (!fileStream.is_open())
			{
				error.Add(std::format("Failed to open file: {}", byteCodeFile.c_str()));
				return false;
			}

			fileStream.write((const char*)this->byteCodeBuffer, this->byteCodeBufferSize);
			fileStream.close();

			if (this->debugInfoDoc)
			{
				std::string jsonString;
				if (!this->debugInfoDoc->PrintJson(jsonString))
				{
					error.Add("Failed to print debug info JSON string.");
					return false;
				}

				std::string debugInfoFile = byteCodeFile.substr(0, byteCodeFile.find_last_of('.')) + ".debuginfo";
				fileStream.open(debugInfoFile, std::fstream::out);
				if (!fileStream.is_open())
				{
					error.Add(std::format("Failed to open file: {}", debugInfoFile.c_str()));
					return false;
				}

				fileStream.write(jsonString.c_str(), jsonString.length());
				fileStream.close();
			}
		}

		return true;
	}

	bool Executable::Load(const std::string& byteCodeFile, Error& error)
	{
		this->Clear();

		std::fstream fileStream;
		fileStream.open(byteCodeFile, std::fstream::in | std::fstream::binary | std::fstream::ate);
		if (!fileStream.is_open())
		{
			error.Add(std::format("Failed to open file: %s", byteCodeFile.c_str()));
			return false;
		}

		this->byteCodeBufferSize = fileStream.tellg();
		if (this->byteCodeBufferSize == 0)
		{
			error.Add(std::format("File (%s) is zero bytes in size.", byteCodeFile.c_str()));
			return false;
		}

		this->byteCodeBuffer = new uint8_t[(uint32_t)this->byteCodeBufferSize];
		fileStream.seekg(0, std::ios::beg);
		fileStream.read((char*)this->byteCodeBuffer, this->byteCodeBufferSize);
		fileStream.close();

		std::string debugInfoFile = byteCodeFile.substr(0, byteCodeFile.find_last_of('.')) + ".debuginfo";
		if (std::filesystem::exists(debugInfoFile))
		{
			fileStream.open(debugInfoFile, std::fstream::in);
			if (!fileStream.is_open())
			{
				error.Add(std::format("Failed to open file: %s", debugInfoFile.c_str()));
				return false;
			}

			std::stringstream stringStream;
			stringStream << fileStream.rdbuf();
			std::string jsonString = stringStream.str();

			std::string parseError;
			ParseParty::JsonValue* jsonValue = ParseParty::JsonValue::ParseJson(jsonString, parseError);
			if (!jsonValue)
			{
				error.Add(std::format("Parse error in JSON file: {}\n\n{}", debugInfoFile.c_str(), parseError.c_str()));
				return false;
			}

			this->debugInfoDoc = dynamic_cast<ParseParty::JsonObject*>(jsonValue);
			if (!this->debugInfoDoc)
			{
				error.Add("Expected debug JSON info to be an object type.");
				return false;
			}
		}

		return true;
	}
}