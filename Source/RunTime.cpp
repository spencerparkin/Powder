#include "RunTime.h"
#include "Value.h"
#include "Exceptions.hpp"
#include "VirtualMachine.h"
#include "Scope.h"
#include "Executable.h"
#include "GarbageCollector.h"
#include "BranchInstruction.h"
#include "ForkInstruction.h"
#include "JumpInstruction.h"
#include "ListInstruction.h"
#include "LoadInstruction.h"
#include "MapInstruction.h"
#include "MathInstruction.h"
#include "PopInstruction.h"
#include "PushInstruction.h"
#include "ScopeInstruction.h"
#include "StoreInstruction.h"
#include "SysCallInstruction.h"
#include "YieldInstruction.h"
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

		this->RegisterInstruction<BranchInstruction>();
		this->RegisterInstruction<ForkInstruction>();
		this->RegisterInstruction<JumpInstruction>();
		this->RegisterInstruction<ListInstruction>();
		this->RegisterInstruction<LoadInstruction>();
		this->RegisterInstruction<MapInstruction>();
		this->RegisterInstruction<MathInstruction>();
		this->RegisterInstruction<PopInstruction>();
		this->RegisterInstruction<PushInstruction>();
		this->RegisterInstruction<ScopeInstruction>();
		this->RegisterInstruction<StoreInstruction>();
		this->RegisterInstruction<SysCallInstruction>();
		this->RegisterInstruction<YieldInstruction>();
	}

	/*virtual*/ RunTime::~RunTime()
	{
		this->UnloadAllModules();
		this->instructionMap.DeleteAndClear();
	}

	void RunTime::ExecuteSourceCodeFile(const std::string& programSourceCodePath, Scope* scope /*= nullptr*/)
	{
		if (!scope)
			scope = this->globalScope.Ptr();

		std::string programSourceCodeResolvedPath = SysCallInstruction::ResolveScriptPath(programSourceCodePath);
		if (!std::filesystem::exists(programSourceCodeResolvedPath))
			throw new RunTimeException(FormatString("Could not find file: %s", programSourceCodePath.c_str()));

		std::string programByteCodePath = programSourceCodeResolvedPath.substr(0, programSourceCodeResolvedPath.find_last_of('.')) + ".pwx";
		if (std::filesystem::exists(programByteCodePath))
		{
			std::filesystem::file_time_type byteCodeTime = std::filesystem::last_write_time(programByteCodePath);
			std::filesystem::file_time_type sourceCodeTime = std::filesystem::last_write_time(programSourceCodeResolvedPath);
			if (byteCodeTime >= sourceCodeTime)
			{
				Executable* executable = new Executable();
				executable->Load(programByteCodePath);
				VirtualMachine vm(this);
				vm.ExecuteByteCode(executable, scope);
				GarbageCollector::GC()->FullPass();
				return;
			}
		}

		std::fstream fileStream;
		fileStream.open(programSourceCodeResolvedPath, std::fstream::in);
		if (!fileStream.is_open())
			throw new RunTimeException(FormatString("Failed to open file: %s", programSourceCodeResolvedPath.c_str()));

		std::stringstream stringStream;
		stringStream << fileStream.rdbuf();
		std::string programCode = stringStream.str();
		fileStream.close();

		this->ExecuteSourceCode(programCode.c_str(), programSourceCodeResolvedPath, scope);
	}

	void RunTime::ExecuteSourceCode(const std::string& programSourceCode, const std::string& programSourceCodePath, Scope* scope /*= nullptr*/)
	{
		if (!scope)
			scope = this->globalScope.Ptr();

		Executable* executable = this->compiler->CompileCode(programSourceCode.c_str());
		if (!executable)
			throw new RunTimeException("Unknown compilation error!");

		if (programSourceCodePath.length() > 0)
		{
			std::string programByteCodePath = programSourceCodePath.substr(0, programSourceCodePath.find_last_of('.')) + ".pwx";
			executable->Save(programByteCodePath);
		}

		VirtualMachine vm(this);
		vm.ExecuteByteCode(executable, scope);
		GarbageCollector::GC()->FullPass();
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

	Instruction* RunTime::LookupInstruction(uint8_t programOpCode)
	{
		char key[2] = { (char)programOpCode, '\0' };
		return this->instructionMap.Lookup(key);
	}
}