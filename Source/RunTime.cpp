#include "RunTime.h"
#include "Value.h"
#include "Exceptions.hpp"
#include "VirtualMachine.h"
#include "Scope.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdint>

namespace Powder
{
	RunTime::RunTime(CompilerInterface* compiler)
	{
		this->compiler = compiler;
		this->globalScope = new Scope();
	}

	/*virtual*/ RunTime::~RunTime()
	{
		this->UnloadAllModules();
	}

	void RunTime::ExecuteSourceCodeFile(const std::string& programSourceCodePath, Scope* scope /*= nullptr*/)
	{
		if (!scope)
			scope = this->globalScope.Ptr();

		if (!std::filesystem::exists(programSourceCodePath))
			throw new RunTimeException(FormatString("File \"%s\" does not exist.", programSourceCodePath.c_str()));

		std::string programByteCodePath = programSourceCodePath.substr(0, programSourceCodePath.find_last_of('.')) + ".pwx";
		if (std::filesystem::exists(programByteCodePath))
		{
			std::filesystem::file_time_type byteCodeTime = std::filesystem::last_write_time(programByteCodePath);
			std::filesystem::file_time_type sourceCodeTime = std::filesystem::last_write_time(programSourceCodePath);
			if (byteCodeTime >= sourceCodeTime)
			{
				std::fstream fileStream;
				fileStream.open(programByteCodePath, std::fstream::in | std::fstream::binary | std::fstream::ate);
				if (!fileStream.is_open())
					throw new RunTimeException(FormatString("Failed to open file: %s", programByteCodePath.c_str()));

				std::streamsize programBufferSize = fileStream.tellg();
				fileStream.seekg(0, std::ios::beg);
				std::vector<char> programBuffer((uint32_t)programBufferSize);
				fileStream.read(programBuffer.data(), programBufferSize);
				fileStream.close();

				VirtualMachine vm(this);
				vm.ExecuteByteCode((uint8_t*)programBuffer.data(), programBufferSize, scope);
				return;
			}
		}

		std::fstream fileStream;
		fileStream.open(programSourceCodePath, std::fstream::in);
		if (!fileStream.is_open())
			throw new RunTimeException(FormatString("Failed to open file: %s", programSourceCodePath.c_str()));

		std::stringstream stringStream;
		stringStream << fileStream.rdbuf();
		std::string programCode = stringStream.str();
		fileStream.close();

		this->ExecuteSourceCode(programCode.c_str(), programSourceCodePath, scope);
	}

	void RunTime::ExecuteSourceCode(const std::string& programSourceCode, const std::string& programSourceCodePath, Scope* scope /*= nullptr*/)
	{
		if (!scope)
			scope = this->globalScope.Ptr();

		uint64_t programBufferSize = 0L;
		uint8_t* programBuffer = this->compiler->CompileCode(programSourceCode.c_str(), programBufferSize);
		if (!programBuffer)
			throw new RunTimeException("Unknown compilation error!");

		if (programSourceCodePath.length() > 0)
		{
			std::string programByteCodePath = programSourceCodePath.substr(0, programSourceCodePath.find_last_of('.')) + ".pwx";
			std::fstream fileStream;
			fileStream.open(programByteCodePath, std::fstream::out | std::fstream::binary);
			if (!fileStream.is_open())
				throw new RunTimeException(FormatString("Failed to open file: %s", programByteCodePath.c_str()));

			fileStream.write((const char*)programBuffer, programBufferSize);
			fileStream.close();
		}

		VirtualMachine vm(this);
		vm.ExecuteByteCode(programBuffer, programBufferSize, scope);
	}

	MapValue* RunTime::LoadModuleFunctionMap(const std::string& moduleAbsolutePath)
	{
		HMODULE moduleHandle = (HMODULE)this->moduleMap.Lookup(moduleAbsolutePath.c_str());
		if (moduleHandle == nullptr)
		{
			moduleHandle = ::LoadLibraryA(moduleAbsolutePath.c_str());
			if (moduleHandle == nullptr)
				throw new RunTimeException(FormatString("No module found at %s", moduleAbsolutePath.c_str()));

			this->moduleMap.Insert(moduleAbsolutePath.c_str(), moduleHandle);
		}

		GenerateFunctionMapFunc generateFunctionMapFunc = (GenerateFunctionMapFunc)::GetProcAddress(moduleHandle, "GenerateFunctionMap");
		if (generateFunctionMapFunc == nullptr)
			throw new RunTimeException(FormatString("Module (%s) does not expose \"GenerateFunctionMap\" function.", moduleAbsolutePath.c_str()));

		return generateFunctionMapFunc();
	}

	void RunTime::UnloadAllModules(void)
	{
		this->moduleMap.ForAllEntries([](const char* key, void* modulePtr) -> bool {
			HMODULE moduleHandle = (HMODULE)modulePtr;
			::FreeLibrary(moduleHandle);
			return true;
		});

		this->moduleMap.Clear();
	}
}