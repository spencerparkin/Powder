#include "VirtualMachine.h"
#include "Executor.h"
#include "Scope.h"
#include "Value.h"
#include "GarbageCollector.h"
#include "BranchInstruction.h"
#include "ForkInstruction.h"
#include "JumpInstruction.h"
#include "LoadInstruction.h"
#include "MathInstruction.h"
#include "PopInstruction.h"
#include "PushInstruction.h"
#include "ScopeInstruction.h"
#include "StoreInstruction.h"
#include "SysCallInstruction.h"
#include "YieldInstruction.h"

namespace Powder
{
	VirtualMachine::VirtualMachine()
	{
		this->executorList = new ExecutorList();
		this->instructionMap = new InstructionMap();

		this->RegisterInstruction<BranchInstruction>();
		this->RegisterInstruction<ForkInstruction>();
		this->RegisterInstruction<JumpInstruction>();
		this->RegisterInstruction<LoadInstruction>();
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
}