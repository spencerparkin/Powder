#include "GarbageCollector.h"
#include "GCObject.h"

namespace Powder
{
	GarbageCollector::GarbageCollector()
	{
		this->visitNumber = 1;
		this->objectSet = new std::set<GCObject*>();
	}

	/*virtual*/ GarbageCollector::~GarbageCollector()
	{
		delete this->objectSet;
	}

	void GarbageCollector::Run()
	{
		if (this->objectSet->size() == 0)
			return;
		
		// Look for an object that can be the start of a BFS for a spanning tree.
		// This is an object we have not yet visited.  Once we find that we have
		// visited them all, we start over.
		std::list<GCObject*> objectQueue;
		while (objectQueue.size() == 0)
		{
			std::set<GCObject*>::iterator setIter = this->objectSet->begin();
			while (true)
			{
				GCObject* object = *setIter;
				if (object->visitNumber != this->visitNumber)
				{
					objectQueue.push_back(object);
					object->visitNumber = this->visitNumber;
					break;
				}

				setIter++;
				if (setIter == this->objectSet->end())
				{
					this->visitNumber++;
					break;
				}
			}
		}

		// Notice that we don't terminate early if we find a reference, because we
		// want to mark all objects in the spanning tree as visited so that the next
		// time the GC is run, we likely don't re-find the same spanning tree.
		bool referenceFound = false;
		std::set<GCObject*> visitationSet;
		while (objectQueue.size() > 0)
		{
			std::list<GCObject*>::iterator listIter = objectQueue.begin();
			GCObject* object = *listIter;
			objectQueue.erase(listIter);

			if (object->IsReference())
				referenceFound = true;

			if (!referenceFound)
				visitationSet.insert(object);

			for (std::set<GCObject*>::iterator setIter = object->adjacencies->begin(); setIter != object->adjacencies->end(); setIter++)
			{
				GCObject* adjacentObject = *setIter;
				if (adjacentObject->visitNumber != this->visitNumber)
				{
					objectQueue.push_back(adjacentObject);
					adjacentObject->visitNumber = this->visitNumber;
				}
			}
		}

		// If the spanning tree did not contain any reference objects, then all objects in that tree can be freed.
		if (!referenceFound)
		{
			for (std::set<GCObject*>::iterator setIter = visitationSet.begin(); setIter != visitationSet.end(); setIter++)
			{
				GCObject* object = *setIter;
				this->Delete(object);		// The destructor for the object will take it out of the object set.
			}
		}
	}

	void GarbageCollector::FullPass()
	{
		this->visitNumber++;
		uint32_t nextVisitNumber = this->visitNumber + 1;
		while (this->visitNumber < nextVisitNumber && this->objectSet->size() > 0)
			this->Run();
	}

	/*virtual*/ void GarbageCollector::Delete(GCObject* object)
	{
		delete object;
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