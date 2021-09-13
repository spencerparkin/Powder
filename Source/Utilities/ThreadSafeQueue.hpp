#pragma once

#include "LinkedList.hpp"
#include <Windows.h>

namespace Powder
{
	template<typename T>
	class ThreadSafeQueue
	{
	public:
		ThreadSafeQueue()
		{
			::InitializeCriticalSection(&this->criticalSection);
		}

		virtual ~ThreadSafeQueue()
		{
			::DeleteCriticalSection(&this->criticalSection);
		}

		void enqueue(T item)
		{
			::EnterCriticalSection(&this->criticalSection);
			this->linkedList.AddTail(item);
			::LeaveCriticalSection(&this->criticalSection);
		}

		bool try_dequeue(T& item)
		{
			if (this->linkedList.GetCount() == 0)
				return false;

			::EnterCriticalSection(&this->criticalSection);
			item = this->linkedList.GetHead()->value;
			this->linkedList.Remove(this->linkedList.GetHead());
			::LeaveCriticalSection(&this->criticalSection);
			return true;
		}

	private:

		LinkedList<T> linkedList;
		CRITICAL_SECTION criticalSection;
	};
}