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

		// This is slow, and so while it should be run periodically, it should
		// not be run every tick of the VM.  I tried to make it fully incremental
		// and quick with each call of run, but there are too many problems with
		// trying to re-enter the algorithm while knowing that the object graph
		// could be changing underneath me.  That said, this is incremental in
		// that it won't necessarily free all outstanding memory that could be
		// reclaimed, but it could take a bit more time than is generally desirable.
		void Run();

		// This runs the GC over all currently known objects.  You might call this
		// at program end to free all remaining memory, which will happen if at the
		// point of being called, all reference objects are out of scope or otherwise
		// deleted.
		void FullPass();

		// This may need to be overridden to prevent us from freeing allocations in
		// the wrong heap.
		virtual void Delete(GCObject* object);

	protected:
		
		std::set<GCObject*>* objectSet;
		uint32_t visitNumber;
	};
}