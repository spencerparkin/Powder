#pragma once

#include "Defines.h"
#include <cstdint>
#include <set>
#include <list>

namespace Powder
{
	class POWDER_API GarbageCollector
	{
		friend class GCObject;
		friend class GCCollectable;

	public:
		GarbageCollector();
		virtual ~GarbageCollector();

		static GarbageCollector* GC();
		static void GC(GarbageCollector* gc);

		// This can and should be run periodically or as needed.
		void Run();

		// This will take the time to purge as much memory as can possibly be
		// reclaimed at the point of being called.
		void FullPurge();

		virtual void Delete(GCObject* object);

		uint32_t RemainingObjectCount();
		uint32_t HonestCollectableCount() { return this->honestCollectableCount; }

	protected:

		void Remember(GCObject* object);
		void Forget(GCObject* object);
		void AdjacenciesChanged(GCObject* object);

		enum State
		{
			STAND_BY,
			INITIATE_BFS,
			CONTINUE_BFS,
			INITIATE_ADJACENCY_WALK,
			CONTINUE_ADJACENCY_WALK,
			CONSUME_SPANNING_TREE_SET
		};

		State state;
		uint32_t unvisitedObjectSet;
		uint32_t visitedObjectSet;
		std::set<GCObject*>* objectSet[2];
		std::set<GCObject*>* spanningTreeSet;
		std::set<GCObject*>* objectQueue;
		std::set<GCObject*>::iterator* objectSetIter;
		GCObject* visitingObject;
		uint32_t visitNumber;
		bool referenceFound;
		uint32_t honestCollectableCount;
	};
}