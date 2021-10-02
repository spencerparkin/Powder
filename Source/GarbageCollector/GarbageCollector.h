#pragma once

#include "Defines.h"
#include "concurrentqueue.h"
#include <stdint.h>
#include "LinkedList.hpp"

namespace Powder
{
	class GCObject;

	// TODO: Detect mis-usage of the GC by marking objects as dead instead of actually deleting them.
	//       Then, when a connection is made with a dead object, we can see that the GC user most likely
	//       tried to transfer ownership of an object without keeping it connected to an anchor at all
	//       times, which produces a race condition where we, if we're fast enough, can sneak in and
	//       delete an object before ownership is transfered.  This is my current guess for why I'm seeing
	//       some crashes where the GC is processing deleted objects.  The GC user thinks the object is
	//       still in scope and connects it something else and that's how it ends up back in our list as
	//       a stale object.
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
		bool CanCollectAll(LinkedList<GCObject*>& objectList);

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
	};
}