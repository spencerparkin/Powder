#pragma once

#include "Defines.h"
#include "concurrentqueue.h"
#include <Windows.h>
#include "LinkedList.hpp"
#include "ThreadSafeQueue.hpp"

namespace Powder
{
	class GCCollectable;

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
		void TrackCollectable(GCCollectable* collectable);
		void RelateCollectables(GCCollectable* collectableA, GCCollectable* collectableB, bool linked);
		uint32_t TrackingCount(void);
		void IncRef(GCCollectable* collectable, uint32_t count);
		void DecRef(GCCollectable* collectable, uint32_t count);

	private:
		static DWORD __stdcall ThreadEntryPoint(LPVOID param);

		void Run(void);
		void UpdateGraph(void);
		void FindSpanningTree(GCCollectable* rootCollectable, LinkedList<GCCollectable*>& spanningTreeList);
		bool CanCollectAll(LinkedList<GCCollectable*>& collectableList);

		HANDLE threadHandle;
		bool threadExitSignaled;
		
		struct GraphModification
		{
			GCCollectable* collectableA;

			union
			{
				GCCollectable* collectableB;
				uint32_t count;
			};
			
			enum Type
			{
				ADD_VERTEX,
				ADD_EDGE,
				DELETE_EDGE,
				INC_REF,
				DEC_REF
			};

			Type type;
		};

#if 1
		typedef moodycamel::ConcurrentQueue<GraphModification> GraphModQueue;
		typedef moodycamel::ConcurrentQueue<GCCollectable*> GarbageQueue;
#else
		typedef ThreadSafeQueue<GraphModification> GraphModQueue;
		typedef ThreadSafeQueue<GCCollectable*> GarbageQueue;
#endif

		GraphModQueue* graphModQueue;
		GarbageQueue* garbageQueue;
		uint32_t spanningTreeKey;
		LinkedList<GCCollectable*> collectableList;
	};
}