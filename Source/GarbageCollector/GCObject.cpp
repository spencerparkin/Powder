#include "GCObject.h"
#include "GarbageCollector.h"

namespace Powder
{
	GCObject::GCObject()
	{
		this->placementNewUsed = false;
		this->visitNumber = 0;
		this->adjacencies = new std::set<GCObject*>();
		GarbageCollector::GC()->Remember(this);
	}
	
	/*virtual*/ GCObject::~GCObject()
	{
		delete this->adjacencies;
		GarbageCollector::GC()->Forget(this);
	}

	void GCObject::ConnectTo(GCObject* object)
	{
		this->adjacencies->insert(object);
		object->adjacencies->insert(this);
		GarbageCollector::GC()->AdjacenciesChanged(object);
		GarbageCollector::GC()->AdjacenciesChanged(this);
	}

	void GCObject::DisconnectFrom(GCObject* object)
	{
		this->adjacencies->erase(object);
		object->adjacencies->erase(this);
		GarbageCollector::GC()->AdjacenciesChanged(object);
		GarbageCollector::GC()->AdjacenciesChanged(this);
	}

	/*static*/ void* GCObject::Allocate(uint64_t size)
	{
		return ::malloc((size_t)size);
	}

	/*static*/ void GCObject::Deallocate(void* memory)
	{
		::free(memory);
	}
}