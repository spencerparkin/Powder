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

	GCObject* GarbageCollector::FindUnvisitedObject()
	{
		// TODO: We really should make this an O(1) operation
		//       by, perhaps, keeping track of which objects
		//       we've visited, and which we haven't, in two
		//       different sets.  A single pass is just a
		//       matter of transfering one set into the other.
		for(GCObject* object : *this->objectSet)
			if (object->visitNumber != this->visitNumber)
				return object;

		return nullptr;
	}

	void GarbageCollector::Run()
	{
		if (this->objectSet->size() == 0)
			return;
		
		std::list<GCObject*> objectQueue;
		GCObject* object = this->FindUnvisitedObject();
		if (!object)
			this->visitNumber++;
		else
			objectQueue.push_back(object);

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
			object->visitNumber = this->visitNumber;

			if (object->IsReference())
				referenceFound = true;

			if (!referenceFound)
				visitationSet.insert(object);

			for (std::set<GCObject*>::iterator setIter = object->adjacencies->begin(); setIter != object->adjacencies->end(); setIter++)
			{
				GCObject* adjacentObject = *setIter;
				if (adjacentObject->visitNumber != this->visitNumber)
					objectQueue.push_back(adjacentObject);
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

	void GarbageCollector::FullPurge()
	{
		uint32_t objectCount = 0;
		do
		{
			objectCount = this->RemainingObjectCount();

			this->visitNumber++;
			uint32_t nextVisitNumber = this->visitNumber + 1;
			while (this->visitNumber < nextVisitNumber && this->objectSet->size() > 0)
				this->Run();

			// We must make another pass if the object count dropped,
			// because this may have also released reference objects
			// that were holding onto collectables that didn't get
			// freed in the previous pass.
		} while (objectCount > this->RemainingObjectCount() && this->objectSet->size() > 0);
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