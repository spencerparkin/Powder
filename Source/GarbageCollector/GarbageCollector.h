#pragma once

#include "Defines.h"
#include "concurrentqueue.h"
#include <Windows.h>
#include "LinkedList.hpp"
#include "ThreadSafeQueue.hpp"

namespace Powder
{
	class GCObject;

	class POWDER_API GarbageCollector
	{
	public:
		GarbageCollector();
		virtual ~GarbageCollector();

		static GarbageCollector* GC();

		bool Startup(void);
		bool Shutdown(void);
		void StallUntilCaughtUp(void);
		void FreeObjects(void);
		void AddObject(GCObject* object);
		void RemoveObject(GCObject* object);
		void RelateObjects(GCObject* objectA, GCObject* objectB, bool linked);
		uint32_t ObjectCount(void);

	private:
		static DWORD __stdcall ThreadEntryPoint(LPVOID param);

		void Run(void);
		bool UpdateGraph(void);
		void FindSpanningTree(GCObject* rootObject, LinkedList<GCObject*>& spanningTreeList, bool removeFromObjectList);
		bool CanCollectAll(LinkedList<GCObject*>& objectList);

		HANDLE threadHandle;
		bool threadExitSignaled;
		
		struct GraphModification
		{
			GCObject* objectA;
			GCObject* objectB;
			
			enum Type
			{
				ADD_VERTEX,
				DEL_VERTEX,
				ADD_EDGE,
				DEL_EDGE
			};

			Type type;
		};

#if 1
		typedef moodycamel::ConcurrentQueue<GraphModification> GraphModQueue;
		typedef moodycamel::ConcurrentQueue<GCObject*> GarbageQueue;
#else
		typedef ThreadSafeQueue<GraphModification> GraphModQueue;
		typedef ThreadSafeQueue<GCObject*> GarbageQueue;
#endif

		GraphModQueue* graphModQueue;
		GarbageQueue* garbageQueue;
		uint32_t spanningTreeKey;
		LinkedList<GCObject*> objectList;
		uint32_t workCount;
		uint32_t targetWorkCount;
		HANDLE caughtUpSemaphore;
	};
}