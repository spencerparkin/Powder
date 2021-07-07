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

	protected:

		void Remember(GCObject* object);
		void Forget(GCObject* object);

		uint32_t unvisitedObjectSet;
		uint32_t visitedObjectSet;
		std::set<GCObject*>* objectSet[2];
		uint32_t visitNumber;
	};
}