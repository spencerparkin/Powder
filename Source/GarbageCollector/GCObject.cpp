#include "GCObject.h"
#include "GarbageCollector.h"

namespace Powder
{
	GCObject::GCObject()
	{
		this->spanningTreeKey = 0;
		this->node = nullptr;
		this->adjacencySet = new std::set<GCObject*>();

		// This should be a lock-free operation.
		GarbageCollector::GC()->AddObject(this);
	}

	/*virtual*/ GCObject::~GCObject()
	{
		// Notice that we do not remove ourselves from the GC system when we go out of scope!
		delete this->adjacencySet;
	}

	void GCObject::ConnectTo(GCObject* object)
	{
		GarbageCollector::GC()->RelateObjects(this, object, true);
	}

	void GCObject::DisconnectFrom(GCObject* object)
	{
		GarbageCollector::GC()->RelateObjects(this, object, false);
	}
}