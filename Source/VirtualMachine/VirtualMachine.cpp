#include "VirtualMachine.h"
#include "Executor.h"
#include "Scope.h"
#include "GarbageCollector.h"
#include "BranchInstruction.h"
#include "ForkInstruction.h"
#include "JumpInstruction.h"
#include "MathInstruction.h"
#include "PopInstruction.h"
#include "PushInstruction.h"
#include "SysCallInstruction.h"
#include "YieldInstruction.h"

namespace Powder
{
	VirtualMachine::VirtualMachine()
	{
		this->globalScope = new Scope(nullptr);
		this->executorList = new ExecutorList();
		this->instructionMap = new InstructionMap();

		this->RegisterInstruction<BranchInstruction>();
		this->RegisterInstruction<ForkInstruction>();
		this->RegisterInstruction<JumpInstruction>();
		this->RegisterInstruction<MathInstruction>();
		this->RegisterInstruction<PopInstruction>();
		this->RegisterInstruction<PushInstruction>();
		this->RegisterInstruction<SysCallInstruction>();
		this->RegisterInstruction<YieldInstruction>();
	}

	/*virtual*/ VirtualMachine::~VirtualMachine()
	{
		this->instructionMap->DeleteAndClear();

		while (this->executorList->size() > 0)
		{
			ExecutorList::iterator iter = this->executorList->begin();
			Executor* executor = *iter;
			delete executor;
			this->executorList->erase(iter);
		}

		delete this->instructionMap;
		delete this->executorList;
		delete this->globalScope;
	}

	void VirtualMachine::CreateExecutorAtLocation(uint64_t programBufferLocation)
	{
		Executor* executor = new Executor(programBufferLocation, this->globalScope);
		this->executorList->push_back(executor);
	}

	/*virtual*/ void VirtualMachine::Execute(uint8_t* programBuffer, uint64_t programBufferSize)
	{
		this->CreateExecutorAtLocation(0);

		while (this->executorList->size() > 0)
		{
			ExecutorList::iterator iter = this->executorList->begin();
			Executor* executor = *iter;
			this->executorList->erase(iter);

			Executor::Result result = executor->Execute(programBuffer, programBufferSize, this);

			if (result == Executor::Result::YIELD)
				this->executorList->push_back(executor);
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