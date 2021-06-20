#include "VirtualMachine.h"
#include "Executor.h"
#include "Scope.h"
#include "Value.h"
#include "ExtensionModule.h"
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
		this->executorList = new ExecutorList();
		this->instructionMap = new InstructionMap();

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
		this->instructionMap->DeleteAndClear();
		DeleteList<Executor*>(*this->executorList);
		delete this->instructionMap;
		delete this->executorList;
		this->UnloadAllExtensionModules();
	}

	void VirtualMachine::CreateExecutorAtLocation(uint64_t programBufferLocation, Executor* forkOrigin /*= nullptr*/)
	{
		Executor* executor = new Executor(programBufferLocation, forkOrigin);
		this->executorList->AddTail(executor);
	}

	/*virtual*/ void VirtualMachine::Execute(uint8_t* programBuffer, uint64_t programBufferSize)
	{
		if (!programBuffer || programBufferSize == 0)
			return;

		this->CreateExecutorAtLocation(0);

		while (this->executorList->GetCount() > 0)
		{
			ExecutorList::Node* node = this->executorList->GetHead();
			Executor* executor = node->value;
			this->executorList->Remove(node);

			Executor::Result result = executor->Execute(programBuffer, programBufferSize, this);

			if (result == Executor::Result::YIELD)
				this->executorList->AddTail(executor);
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
		return this->instructionMap->Lookup(key);
	}

	ExtensionModule::Function* VirtualMachine::LookupModuleFunction(const std::string& funcName)
	{
		return this->extensionFunctionMap.Lookup(funcName.c_str());
	}

	void VirtualMachine::LoadExtensionModule(const std::string& modulePath)
	{
		LoadedExtensionModule loadedExtensionModule;

		loadedExtensionModule.moduleHandle = ::LoadLibraryA(modulePath.c_str());
		if (loadedExtensionModule.moduleHandle == NULL)
			throw new Exception(FormatString("Failed to load extension module: %s", modulePath.c_str()));

		const char* registerFuncName = "RegisterExtensionModule";
		RegisterExtensionModuleProc registerProc = (RegisterExtensionModuleProc)::GetProcAddress((HMODULE)loadedExtensionModule.moduleHandle, registerFuncName);
		if (!registerProc)
			throw new Exception(FormatString("Failed to find proc-address \"%s\" for module: %s", registerFuncName, modulePath.c_str()));

		loadedExtensionModule.moduleInstance = registerProc();
		if (!loadedExtensionModule.moduleInstance)
			throw new Exception(FormatString("Failed to get module instance from module: %s", modulePath.c_str()));

		loadedExtensionModule.moduleInstance->RegisterFunctions(this->extensionFunctionMap);
		this->loadedExtensionModuleList.AddTail(loadedExtensionModule);
	}

	void VirtualMachine::UnloadAllExtensionModules(void)
	{
		while (this->loadedExtensionModuleList.GetCount() > 0)
		{
			LoadedExtensionModule& loadedExtensionModule = this->loadedExtensionModuleList.GetHead()->value;
			::FreeLibrary((HMODULE)loadedExtensionModule.moduleHandle);
			this->loadedExtensionModuleList.Remove(this->loadedExtensionModuleList.GetHead());
		}

		this->extensionFunctionMap.Clear();
	}
}