#include "VirtualMachine.h"
#include "Executor.h"
#include "Exceptions.hpp"
#include "Executable.h"

namespace Powder
{
	VirtualMachine::VirtualMachine(RunTime* runTime)
	{
		this->runTime = runTime;
	}

	/*virtual*/ VirtualMachine::~VirtualMachine()
	{
		DeleteList<Executor*>(this->executorList);
	}

	void VirtualMachine::CreateExecutorAtLocation(uint64_t programBufferLocation, Scope* scope)
	{
		Executor* executor = new Executor(programBufferLocation, scope);
		this->executorList.AddTail(executor);
	}

	void VirtualMachine::ExecuteByteCode(const Executable* executable, Scope* scope)
	{
		GCReference<Executable> exectuableRef(const_cast<Executable*>(executable));

		this->CreateExecutorAtLocation(0L, scope);

		while (this->executorList.GetCount() > 0)
		{
			ExecutorList::Node* node = this->executorList.GetHead();
			Executor* executor = node->value;
			this->executorList.Remove(node);

			Executor::Result result = executor->Execute(executable, this);

			if (result == Executor::Result::YIELD)
				this->executorList.AddTail(executor);
			else if (result == Executor::Result::HALT)
				delete executor;
			else
				break;
		}
	}
}