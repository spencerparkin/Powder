#include "VirtualMachine.h"
#include "Executor.h"
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
#include "Compiler.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>

namespace Powder
{
	VirtualMachine::IODevice theDefaultIODevice;

	VirtualMachine::VirtualMachine()
	{
		this->compiler = &theDefaultCompiler;
		this->ioDevice = &theDefaultIODevice;
		this->debuggerTrap = nullptr;
		
		this->globalScopeRef.Set(new Scope());
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

	bool VirtualMachine::CreateExecutorAtLocation(uint64_t programBufferLocation, const Executable* executable, Scope* scope, Error& error)
	{
		if (this->executorListStack->size() == 0)
		{
			error.Add("Executor list stack size is zero when trying to create new executor");
			return false;
		}

		if (programBufferLocation >= executable->byteCodeBufferSize)
		{
			error.Add("Given program buffer location matches or exceeds the size of the given executable buffer.");
			return false;
		}

		ExecutorList* executorList = (*this->executorListStack)[this->executorListStack->size() - 1];
		Executor* executor = new Executor(programBufferLocation, executable, scope);
		executorList->AddTail(executor);
		return true;
	}

	bool VirtualMachine::ExecuteByteCode(GC::Reference<Executor, true>& executorRef, Error& error)
	{
		ExecutorList executorList;
		this->executorListStack->push_back(&executorList);
		executorList.AddTail(executorRef.Get());
		
		Executor::Result result;

		while (executorList.GetCount() > 0)
		{
			ExecutorList::Node* node = executorList.GetHead();
			GC::Reference<Executor, true> executorRef(node->value.Get());
			result = executorRef.Get()->Execute(this, error);
			executorList.Remove(node);
			if (result == Executor::Result::YIELD)
				executorList.AddTail(executorRef.Get());
			else if (result == Executor::Result::RUNTIME_ERROR)
				break;
		}

		executorList.RemoveAll();
		this->executorListStack->pop_back();

		return result != Executor::Result::RUNTIME_ERROR;
	}

	bool VirtualMachine::ExecuteSourceCodeFile(const std::string& programSourceCodePath, Error& error, Scope* scope /*= nullptr*/)
	{
		if (!scope)
			scope = this->globalScopeRef.Get();

		std::string programSourceCodeResolvedPath = pathResolver.ResolvePath(programSourceCodePath, PathResolver::SEARCH_CWD, error);
		if (programSourceCodeResolvedPath.size() == 0)
			return false;
		std::string programByteCodePath = programSourceCodeResolvedPath.substr(0, programSourceCodeResolvedPath.find_last_of('.')) + ".pwx";
		if (std::filesystem::exists(programByteCodePath))
		{
			std::filesystem::file_time_type byteCodeTime = std::filesystem::last_write_time(programByteCodePath);
			std::filesystem::file_time_type sourceCodeTime = std::filesystem::last_write_time(programSourceCodeResolvedPath);
			if (byteCodeTime >= sourceCodeTime)
			{
				GC::Reference<Executable, true> executableRef(new Executable());
				if (!executableRef.Get()->Load(programByteCodePath, error))
					return false;

				GC::Reference<Executor, true> executorRef(new Executor(0L, executableRef.Get(), scope));
				if (!this->ExecuteByteCode(executorRef, error))
					return false;
				
				return true;
			}
		}

		std::fstream fileStream;
		fileStream.open(programSourceCodeResolvedPath, std::fstream::in);
		if (!fileStream.is_open())
		{
			error.Add(std::format("Failed to open file: {}", programSourceCodeResolvedPath.c_str()));
			return false;
		}

		std::stringstream stringStream;
		stringStream << fileStream.rdbuf();
		std::string programCode = stringStream.str();
		fileStream.close();

		return this->ExecuteSourceCode(programCode, programSourceCodeResolvedPath, error, scope);
	}

	bool VirtualMachine::ExecuteSourceCode(const std::string& programSourceCode, const std::string& programSourceCodePath, Error& error, Scope* scope /*= nullptr*/)
	{
		if (!scope)
			scope = this->globalScopeRef.Get();

		GC::Reference<Executable, true> executableRef(this->compiler->CompileCode(programSourceCode.c_str(), error));
		if (!executableRef.Get())
		{
			error.Add("Compilation failed!");
			return false;
		}

		if (programSourceCodePath.length() > 0)
		{
			if (executableRef.Get()->debugInfoDoc)
				executableRef.Get()->debugInfoDoc->SetValue("source_file", new ParseParty::JsonString(programSourceCodePath.c_str()));

			std::string programByteCodePath = programSourceCodePath.substr(0, programSourceCodePath.find_last_of('.')) + ".pwx";
			if (!executableRef.Get()->Save(programByteCodePath, error))
			{
				error.Add("Failed to save executable to disk at: " + programByteCodePath);
				return false;
			}
		}

		GC::Reference<Executor, true> executorRef(new Executor(0L, executableRef.Get(), scope));
		return this->ExecuteByteCode(executorRef, error);
	}

	MapValue* VirtualMachine::LoadModuleFunctionMap(const std::string& moduleAbsolutePath, Error& error)
	{
		HMODULE moduleHandle = (HMODULE)this->moduleMap.Lookup(moduleAbsolutePath.c_str());
		if (moduleHandle == nullptr)
		{
			moduleHandle = ::LoadLibraryA(moduleAbsolutePath.c_str());
			if (moduleHandle == nullptr)
			{
				error.Add(std::format("No module found at {}.", moduleAbsolutePath.c_str()));
				return nullptr;
			}

			this->moduleMap.Insert(moduleAbsolutePath.c_str(), moduleHandle);
		}

		GenerateFunctionMapFunc generateFunctionMapFunc = (GenerateFunctionMapFunc)::GetProcAddress(moduleHandle, "GenerateFunctionMap");
		if (generateFunctionMapFunc == nullptr)
		{
			error.Add(std::format("Module ({}) does not expose \"GenerateFunctionMap\" function.", moduleAbsolutePath.c_str()));
			return nullptr;
		}

		return generateFunctionMapFunc();
	}

	void VirtualMachine::UnloadAllModules(void)
	{
		// We must first purge the GC system of any objects that may
		// have virtual functions in a module we're about to unload!
		this->globalScopeRef.Get()->GetValueMap()->Clear();
		GC::GarbageCollector::Get()->Collect();

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

	void VirtualMachine::GetAllCurrentScopes(std::vector<Scope*>& scopeArray)
	{
		for (int i = 0; i < (signed)this->executorListStack->size(); i++)
		{
			ExecutorList* executorList = (*this->executorListStack)[i];
			for(ExecutorList::Node* node = executorList->GetHead(); node; node = node->GetNext())
			{
				Executor* executor = node->value.Get();
				Scope* scope = executor->GetCurrentScope();
				scopeArray.push_back(scope);
			}
		}
	}

	VirtualMachine::CompilerInterface::CompilerInterface()
	{
	}

	/*virtual*/ VirtualMachine::CompilerInterface::~CompilerInterface()
	{
	}

	VirtualMachine::IODevice::IODevice()
	{
	}

	/*virtual*/ VirtualMachine::IODevice::~IODevice()
	{
	}

	/*virtual*/ void VirtualMachine::IODevice::InputString(std::string& str)
	{
		std::getline(std::cin, str);
	}

	/*virtual*/ void VirtualMachine::IODevice::OutputString(const std::string& str)
	{
		std::cout << str;
	}

	VirtualMachine::DebuggerTrap::DebuggerTrap()
	{
	}

	/*virtual*/ VirtualMachine::DebuggerTrap::~DebuggerTrap()
	{
	}
}