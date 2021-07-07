#include "GarbageCollector.h"
#include "GCObject.h"
#include <assert.h>

namespace Powder
{
	GarbageCollector::GarbageCollector()
	{
		this->visitNumber = 0;
		this->unvisitedObjectSet = 0;
		this->visitedObjectSet = 1;
		this->objectSet[0] = new std::set<GCObject*>();
		this->objectSet[1] = new std::set<GCObject*>();
	}

	/*virtual*/ GarbageCollector::~GarbageCollector()
	{
		delete this->objectSet[0];
		delete this->objectSet[1];
	}

	void GarbageCollector::Remember(GCObject* object)
	{
		this->objectSet[this->unvisitedObjectSet]->insert(object);
	}

	void GarbageCollector::Forget(GCObject* object)
	{
		std::set<GCObject*>::iterator iter = this->objectSet[this->visitedObjectSet]->find(object);
		if (iter != this->objectSet[this->visitedObjectSet]->end())
			this->objectSet[this->visitedObjectSet]->erase(iter);
		else
		{
			iter = this->objectSet[this->unvisitedObjectSet]->find(object);
			if (iter != this->objectSet[this->unvisitedObjectSet]->end())
				this->objectSet[this->unvisitedObjectSet]->erase(iter);
			else
			{
				assert(false);
			}
		}
	}

	void GarbageCollector::Run()
	{
		if (this->RemainingObjectCount() == 0)
			return;
		
		if (this->objectSet[this->unvisitedObjectSet]->size() == 0)
		{
			this->unvisitedObjectSet = 1 - this->unvisitedObjectSet;
			this->visitedObjectSet = 1 - this->visitedObjectSet;
		}

		// It's important that we find the full spanning tree here rather
		// than bail early, because we don't want to re-find the same tree
		// more than once in a single pass over all objects.
		bool referenceFound = false;
		std::set<GCObject*> spanningTreeSet;
		std::list<GCObject*> objectQueue;
		GCObject* object = *this->objectSet[this->unvisitedObjectSet]->begin();
		object->visitNumber = ++this->visitNumber;
		objectQueue.push_back(object);
		while (objectQueue.size() > 0)
		{
			std::list<GCObject*>::iterator queueIter = objectQueue.begin();
			object = *queueIter;
			objectQueue.erase(queueIter);
			
			this->objectSet[this->unvisitedObjectSet]->erase(object);
			this->objectSet[this->visitedObjectSet]->insert(object);

			if (object->IsReference())
				referenceFound = true;

			if (!referenceFound)
				spanningTreeSet.insert(object);

			for (GCObject* adjacentObject : *object->adjacencies)
			{
				// Is the adjacent object destined for visitation or has it already been visited?
				if (adjacentObject->visitNumber != this->visitNumber)
				{
					objectQueue.push_back(adjacentObject);
					adjacentObject->visitNumber = this->visitNumber;
				}
			}
		}

		// If the spanning tree did not contain any reference objects, then all objects in the tree can be freed.
		if (!referenceFound)
			for(GCObject* object : spanningTreeSet)
				this->Delete(object);		// The destructor for the object will take it out of the GC graph.
	}

	void GarbageCollector::FullPurge()
	{
		// Finish our current pass, if any.
		while (this->objectSet[this->unvisitedObjectSet]->size() > 0)
			this->Run();

		// Note that the goal here is to purge as much as is currently possible,
		// which is not necessarily all outstanding objects.
		while (true)
		{
			// If all objects have been purged, we're obviously done.
			uint32_t remainingObjectCount = this->RemainingObjectCount();
			if (remainingObjectCount == 0)
				break;

			// Complete one full pass over the remaining objects.
			this->Run();
			while (this->objectSet[this->unvisitedObjectSet]->size() > 0)
				this->Run();

			// If nothing was purged, we're done.  If anything was purged,
			// we must go for another pass, because the previous pass may
			// have removed references holding onto other collectables.
			if (this->RemainingObjectCount() == remainingObjectCount)
				break;
		}
	}

	/*virtual*/ void GarbageCollector::Delete(GCObject* object)
	{
		if (!object->placementNewUsed)
			delete object;
		else
		{
			object->~GCObject();
			GCObject::Deallocate(object);
		}
	}

	uint32_t GarbageCollector::RemainingObjectCount()
	{
		uint32_t unvisitedObjectCount = this->objectSet[this->unvisitedObjectSet]->size();
		uint32_t visitedObjectCount = this->objectSet[this->visitedObjectSet]->size();
		return unvisitedObjectCount + visitedObjectCount;
	}

	static GarbageCollector defaultGC;
	static GarbageCollector* theGC = &defaultGC;

	/*static*/ GarbageCollector* GarbageCollector::GC()
	{
		return theGC;
	}

	/*static*/ void GarbageCollector::GC(GarbageCollector* gc)
	{
		theGC = gc;
	}
}