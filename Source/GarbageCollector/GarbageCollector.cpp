#include "GarbageCollector.h"
#include "GCObject.h"

namespace Powder
{
	GarbageCollector::GarbageCollector()
	{
		this->honestCollectableCount = 0;
		this->state = STAND_BY;
		this->visitNumber = 0;
		this->referenceFound = false;
		this->visitingObject = nullptr;
		this->unvisitedObjectSet = 0;
		this->visitedObjectSet = 1;
		this->objectSet[0] = new std::set<GCObject*>();
		this->objectSet[1] = new std::set<GCObject*>();
		this->spanningTreeSet = new std::set<GCObject*>();
		this->objectQueue = new std::set<GCObject*>();
		this->objectSetIter = new std::set<GCObject*>::iterator();
	}

	/*virtual*/ GarbageCollector::~GarbageCollector()
	{
		delete this->objectSet[0];
		delete this->objectSet[1];
		delete this->spanningTreeSet;
		delete this->objectQueue;
		delete this->objectSetIter;
	}

	void GarbageCollector::Remember(GCObject* object)
	{
		this->objectSet[this->unvisitedObjectSet]->insert(object);
	}

	void GarbageCollector::Forget(GCObject* object)
	{
		this->objectSet[this->visitedObjectSet]->erase(object);
		this->objectSet[this->unvisitedObjectSet]->erase(object);
		this->objectQueue->erase(object);
		this->spanningTreeSet->erase(object);
		if (this->visitingObject == object)
		{
			this->visitingObject = nullptr;
			this->state = STAND_BY;
		}
	}

	void GarbageCollector::AdjacenciesChanged(GCObject* object)
	{
		if (this->state != STAND_BY)
		{
			std::set<GCObject*>::iterator iter = this->spanningTreeSet->find(object);
			if (iter != this->spanningTreeSet->end())
				this->state = STAND_BY;

			iter = this->objectQueue->find(object);
			if (iter != this->objectQueue->end())
				this->state = STAND_BY;
		}
	}

	void GarbageCollector::Run()
	{
		switch (this->state)
		{
			case STAND_BY:
			{
				if (this->RemainingObjectCount() == 0)
					return;

				this->state = INITIATE_BFS;
				break;
			}
			case INITIATE_BFS:
			{
				this->spanningTreeSet->clear();

				if (this->objectSet[this->unvisitedObjectSet]->size() == 0)
				{
					this->unvisitedObjectSet = 1 - this->unvisitedObjectSet;
					this->visitedObjectSet = 1 - this->visitedObjectSet;
				}

				if (this->objectSet[this->unvisitedObjectSet]->size() == 0)
					this->state = STAND_BY;
				else
				{
					// A random choice here might be better, but just use the first one for now.
					GCObject* object = *this->objectSet[this->unvisitedObjectSet]->begin();
					object->visitNumber = ++this->visitNumber;
					this->objectQueue->insert(object);
					this->referenceFound = false;
					this->state = CONTINUE_BFS;
				}

				break;
			}
			case CONTINUE_BFS:
			{
				if (this->objectQueue->size() == 0)
				{
					this->visitingObject = nullptr;
					this->state = CONSUME_SPANNING_TREE_SET;
				}
				else
				{
					std::set<GCObject*>::iterator queueIter = this->objectQueue->begin();
					this->visitingObject = *queueIter;
					this->objectQueue->erase(queueIter);

					if (this->visitingObject->IsReference())
						this->referenceFound = true;

					this->spanningTreeSet->insert(this->visitingObject);
					this->state = INITIATE_ADJACENCY_WALK;
				}

				break;
			}
			case INITIATE_ADJACENCY_WALK:
			{
				*this->objectSetIter = this->visitingObject->adjacencies->begin();
				this->state = CONTINUE_ADJACENCY_WALK;
				break;
			}
			case CONTINUE_ADJACENCY_WALK:
			{
				if (*this->objectSetIter == this->visitingObject->adjacencies->end())
					this->state = CONTINUE_BFS;
				else
				{
					GCObject* adjacentObject = **objectSetIter;
					
					// Is the adjacent object destined for visitation or has it already been visited?
					if (adjacentObject->visitNumber != this->visitNumber)
					{
						this->objectQueue->insert(adjacentObject);
						adjacentObject->visitNumber = this->visitNumber;
					}

					(*this->objectSetIter)++;
				}

				break;
			}
			case CONSUME_SPANNING_TREE_SET:
			{
				if (this->spanningTreeSet->size() == 0)
					this->state = STAND_BY;
				else
				{
					std::set<GCObject*>::iterator iter = this->spanningTreeSet->begin();
					GCObject* object = *iter;
					this->spanningTreeSet->erase(iter);

					this->objectSet[this->unvisitedObjectSet]->erase(object);
					this->objectSet[this->visitedObjectSet]->insert(object);

					if (!this->referenceFound)
						this->Delete(object);	// The destructor for the object will take it out of the GC graph.
				}

				break;
			}
		}
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
			while (this->objectSet[this->unvisitedObjectSet]->size() == 0)
				this->Run();
			while (this->objectSet[this->unvisitedObjectSet]->size() > 0)
				this->Run();

			// If nothing was purged, we're done.  If anything was purged,
			// we must go for another pass, because the previous pass may
			// have removed references holding onto other collectables,
			// not because we delete references (we don't, the user must
			// manage that memory), but because collectables can own references.
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