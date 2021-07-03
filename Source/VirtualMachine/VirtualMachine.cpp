#include "VirtualMachine.h"
#include "Executor.h"
#include "Scope.h"
#include "Value.h"
#include "Exceptions.hpp"
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

namespace Powder
{
	VirtualMachine::VirtualMachine()
	{
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
		this->instructionMap.DeleteAndClear();
		DeleteList<Executor*>(this->executorList);
		this->UnloadAllModules();
	}

	void VirtualMachine::CreateExecutorAtLocation(uint64_t programBufferLocation, Executor* forkOrigin /*= nullptr*/)
	{
		Executor* executor = new Executor(programBufferLocation, forkOrigin);
		this->executorList.AddTail(executor);
	}

	/*virtual*/ void VirtualMachine::Execute(uint8_t* programBuffer, uint64_t programBufferSize)
	{
		if (!programBuffer || programBufferSize == 0)
			return;

		this->CreateExecutorAtLocation(0);

		while (this->executorList.GetCount() > 0)
		{
			ExecutorList::Node* node = this->executorList.GetHead();
			Executor* executor = node->value;
			this->executorList.Remove(node);

			Executor::Result result = executor->Execute(programBuffer, programBufferSize, this);

			if (result == Executor::Result::YIELD)
				this->executorList.AddTail(executor);
			else if (result == Executor::Result::HALT)
				delete executor;
			else
				break;
		}

		GarbageCollector::GC()->FullPass();
	}

	Instruction* VirtualMachine::LookupInstruction(uint8_t programOpCode)
	{
		char key[2] = { (char)programOpCode, '\0' };
		return this->instructionMap.Lookup(key);
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
		this->moduleMap.ForAllEntries([](const char* key, void* modulePtr) -> bool {
			HMODULE moduleHandle = (HMODULE)modulePtr;
			::FreeLibrary(moduleHandle);
			return true;
		});

		this->moduleMap.Clear();
	}
}