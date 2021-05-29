#include "GCObject.h"
#include "GarbageCollector.h"

namespace Powder
{
	GCObject::GCObject()
	{
		this->visitNumber = 0;
		this->adjacencies = new std::set<GCObject*>();
		GarbageCollector::GC()->objectSet->insert(this);
	}
	
	/*virtual*/ GCObject::~GCObject()
	{
		delete this->adjacencies;
		GarbageCollector::GC()->objectSet->erase(this);
	}

	void GCObject::ConnectTo(GCObject* object)
	{
		this->adjacencies->insert(object);
		object->adjacencies->insert(this);
	}

	void GCObject::DisconnectFrom(GCObject* object)
	{
		this->adjacencies->erase(object);
		object->adjacencies->erase(this);
	}
}