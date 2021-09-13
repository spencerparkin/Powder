#pragma once

#include "Defines.h"
#include "concurrentqueue.h"
#include <Windows.h>
#include "LinkedList.hpp"

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

	private:
		static DWORD __stdcall ThreadEntryPoint(LPVOID param);

		void Run(void);
		void UpdateGraph(void);
		GCCollectable* FindUnvisitedCollectable(void);
		void FindSpanningTree(GCCollectable* rootCollectable, LinkedList<GCCollectable*>& spanningTreeList);
		bool CanCollectAll(LinkedList<GCCollectable*>& collectableList);

		HANDLE threadHandle;
		bool threadExitSignaled;
		
		struct GraphModification
		{
			GCCollectable* collectableA;
			GCCollectable* collectableB;
			
			enum Type
			{
				ADD_VERTEX,
				ADD_EDGE,
				DELETE_EDGE
			};

			Type type;
		};

		moodycamel::ConcurrentQueue<GraphModification>* graphModQueue;
		moodycamel::ConcurrentQueue<GCCollectable*>* garbageQueue;
		uint32_t spanningTreeKey;
		LinkedList<GCCollectable*> collectableList;
	};
}