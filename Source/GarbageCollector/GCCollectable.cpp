#include "GCCollectable.h"
#include "GarbageCollector.h"

namespace Powder
{
	GCCollectable::GCCollectable()
	{
		this->refCount = 1;
		this->spanningTreeKey = 0;
		this->node = nullptr;
		this->adjacencySet = new std::set<GCCollectable*>();

		// This should be a lock-free operation.
		GarbageCollector::GC()->TrackCollectable(this);
	}

	/*virtual*/ GCCollectable::~GCCollectable()
	{
		delete this->adjacencySet;
	}

	void GCCollectable::ConnectTo(GCCollectable* collectable)
	{
		GarbageCollector::GC()->RelateCollectables(this, collectable, true);
	}

	void GCCollectable::DisconnectFrom(GCCollectable* collectable)
	{
		GarbageCollector::GC()->RelateCollectables(this, collectable, false);
	}

	void GCCollectable::IncRef()
	{
		this->refCount++;
	}

	void GCCollectable::DecRef()
	{
		assert(this->refCount > 0);
		this->refCount--;
	}
}