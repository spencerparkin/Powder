#include "GCObject.h"
#include "GarbageCollector.h"

namespace Powder
{
	GCObject::GCObject()
	{
		this->spanningTreeKey = 0;
		this->node = nullptr;
		this->adjacencyMap = new std::map<GCObject*, uint32_t>();
		this->marked = false;
#if defined GC_DEBUG
		this->deleted = false;
#endif

		// This should be a lock-free operation.
		GarbageCollector::GC()->AddObject(this);
	}

	/*virtual*/ GCObject::~GCObject()
	{
		// Notice that we do not remove ourselves from the GC system when we go out of scope!
		delete this->adjacencyMap;
	}

	void GCObject::ConnectTo(GCObject* object)
	{
		GarbageCollector::GC()->RelateObjects(this, object, true);
	}

	void GCObject::DisconnectFrom(GCObject* object)
	{
		GarbageCollector::GC()->RelateObjects(this, object, false);
	}

	void GCObject::AddAdjacency(GCObject* object)
	{
		std::map<GCObject*, uint32_t>::iterator iter = this->adjacencyMap->find(object);
		if (iter != this->adjacencyMap->end())
			iter->second++;
		else
			this->adjacencyMap->insert(std::pair<GCObject*, uint32_t>(object, 1));
	}

	bool GCObject::RemoveAdjacency(GCObject* object)
	{
		std::map<GCObject*, uint32_t>::iterator iter = this->adjacencyMap->find(object);
		if (iter == this->adjacencyMap->end())
			return false;
		if (--iter->second == 0)
			this->adjacencyMap->erase(iter);
		return true;
	}
}