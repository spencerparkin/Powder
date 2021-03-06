#pragma once

#include "Defines.h"
#include "concurrentqueue.h"
#include <stdint.h>
#include "LinkedList.hpp"

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
		static unsigned long __stdcall ThreadEntryPoint(void* param);

		void Run(void);
		bool UpdateGraph(void);
		void FindSpanningTree(GCObject* rootObject, LinkedList<GCObject*>& spanningTreeList, bool removeFromObjectList);
		bool CanCollectAll(const LinkedList<GCObject*>& givenObjectList);

		void* threadHandle;
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

		typedef moodycamel::ConcurrentQueue<GraphModification> GraphModQueue;
		typedef moodycamel::ConcurrentQueue<GCObject*> GarbageQueue;

		GraphModQueue* graphModQueue;
		GarbageQueue* garbageQueue;
		uint32_t spanningTreeKey;
		LinkedList<GCObject*> objectList;
		uint32_t workCount;
		uint32_t targetWorkCount;
		void* caughtUpSemaphore;
#if defined GC_DEBUG
		GCObject* debugObject;
#endif
	};
}