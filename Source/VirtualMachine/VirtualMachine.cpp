#include "VirtualMachine.h"
#include "Executor.h"
#include "Exceptions.hpp"
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

namespace Powder
{
	VirtualMachine::VirtualMachine(RunTime* runTime)
	{
		this->runTime = runTime;

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
	}

	void VirtualMachine::CreateExecutorAtLocation(uint64_t programBufferLocation, Scope* scope)
	{
		Executor* executor = new Executor(programBufferLocation, scope);
		this->executorList.AddTail(executor);
	}

	void VirtualMachine::ExecuteByteCode(uint8_t* programBuffer, uint64_t programBufferSize, Scope* scope)
	{
		if (!programBuffer || programBufferSize == 0)
			return;

		this->CreateExecutorAtLocation(0L, scope);

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
	}

	Instruction* VirtualMachine::LookupInstruction(uint8_t programOpCode)
	{
		char key[2] = { (char)programOpCode, '\0' };
		return this->instructionMap.Lookup(key);
	}
}