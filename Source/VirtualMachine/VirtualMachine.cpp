#include "VirtualMachine.h"
#include "Executor.h"
#include "Exceptions.hpp"
#include "Executable.h"
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
#include "GarbageCollector.h"
#include "PathResolver.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdint>

namespace Powder
{
	VirtualMachine::VirtualMachine(CompilerInterface* compiler, DebuggerInterface* debugger)
	{
		this->compiler = compiler;
		this->debugger = debugger;
		this->globalScope = new Scope();		// This gets deleted by the GC.
		this->executorListStack = new std::vector<ExecutorList*>();
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

	/*virtual*/ VirtualMachine::~VirtualMachine()
	{
		this->UnloadAllModules();
		this->instructionMap.DeleteAndClear();
		delete this->executorListStack;
	}

	void VirtualMachine::CreateExecutorAtLocation(uint64_t programBufferLocation, Scope* scope)
	{
		if (this->executorListStack->size() == 0)
			throw new RunTimeException("Executor list stack size is zero when trying to create new executor");

		ExecutorList* executorList = (*this->executorListStack)[this->executorListStack->size() - 1];
		Executor* executor = new Executor(programBufferLocation, scope);
		executorList->AddTail(executor);
	}

	void VirtualMachine::ExecuteByteCode(const Executable* executable, Scope* scope)
	{
		GCReference<Executable> exectuableRef(const_cast<Executable*>(executable));

		ExecutorList executorList;
		this->executorListStack->push_back(&executorList);
		executorList.AddTail(new Executor(0L, scope));

		while (executorList.GetCount() > 0)
		{
			ExecutorList::Node* node = executorList.GetHead();
			Executor* executor = node->value;
			executorList.Remove(node);

			Executor::Result result = executor->Execute(executable, this);

			if (result == Executor::Result::YIELD)
				executorList.AddTail(executor);
			else if (result == Executor::Result::HALT)
				delete executor;
			else
				break;
		}

		DeleteList<Executor*>(executorList);
		this->executorListStack->pop_back();
	}

	void VirtualMachine::ExecuteSourceCodeFile(const std::string& programSourceCodePath, Scope* scope /*= nullptr*/)
	{
		if (!scope)
			scope = this->globalScope.Ptr();

		std::string programSourceCodeResolvedPath = pathResolver.ResolvePath(programSourceCodePath, PathResolver::SEARCH_CWD);
		std::string programByteCodePath = programSourceCodeResolvedPath.substr(0, programSourceCodeResolvedPath.find_last_of('.')) + ".pwx";
		if (std::filesystem::exists(programByteCodePath))
		{
			std::filesystem::file_time_type byteCodeTime = std::filesystem::last_write_time(programByteCodePath);
			std::filesystem::file_time_type sourceCodeTime = std::filesystem::last_write_time(programSourceCodeResolvedPath);
			if (byteCodeTime >= sourceCodeTime)
			{
				Executable* executable = new Executable();
				executable->Load(programByteCodePath);
				this->ExecuteByteCode(executable, scope);
				GarbageCollector::GC()->FullPurge();
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

		this->ExecuteSourceCode(programCode, programSourceCodeResolvedPath, scope);
	}

	void VirtualMachine::ExecuteSourceCode(const std::string& programSourceCode, const std::string& programSourceCodePath, Scope* scope /*= nullptr*/)
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

		this->ExecuteByteCode(executable, scope);
		GarbageCollector::GC()->FullPurge();
	}

	MapValue* VirtualMachine::LoadModuleFunctionMap(const std::string& moduleAbsolutePath)
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

	void VirtualMachine::UnloadAllModules(void)
	{
		// We must first purge the GC system of any objects that may
		// have virtual functions in a module we're about to unload!
		this->globalScope.Clear();
		GarbageCollector::GC()->FullPurge();

		this->moduleMap.ForAllEntries([](const char* key, void* modulePtr) -> bool {
			HMODULE moduleHandle = (HMODULE)modulePtr;
			::FreeLibrary(moduleHandle);
			return true;
			});

		this->moduleMap.Clear();
	}

	Instruction* VirtualMachine::LookupInstruction(uint8_t programOpCode)
	{
		char key[2] = { (char)programOpCode, '\0' };
		return this->instructionMap.Lookup(key);
	}
}