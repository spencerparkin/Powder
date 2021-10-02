#include "GarbageCollector.h"
#include "GCCollectable.h"
#include <Windows.h>

namespace Powder
{
	static GarbageCollector theGC;

	GarbageCollector::GarbageCollector()
	{
		this->threadHandle = nullptr;
		this->threadExitSignaled = false;
		this->spanningTreeKey = 0;
		this->graphModQueue = new GraphModQueue();
		this->garbageQueue = new GarbageQueue();
		this->workCount = 0;
		this->targetWorkCount = 0;
		this->caughtUpSemaphore = nullptr;
	}

	/*virtual*/ GarbageCollector::~GarbageCollector()
	{
		delete this->graphModQueue;
		delete this->garbageQueue;
	}

	/*static*/ GarbageCollector* GarbageCollector::GC()
	{
		return &theGC;
	}

	void GarbageCollector::AddObject(GCObject* object)
	{
		GraphModification graphMod;
		graphMod.objectA = object;
		graphMod.objectB = nullptr;
		graphMod.type = GraphModification::ADD_VERTEX;
		this->graphModQueue->enqueue(graphMod);
	}

	void GarbageCollector::RemoveObject(GCObject* object)
	{
		GraphModification graphMod;
		graphMod.objectA = object;
		graphMod.objectB = nullptr;
		graphMod.type = GraphModification::DEL_VERTEX;
		this->graphModQueue->enqueue(graphMod);
	}

	void GarbageCollector::RelateObjects(GCObject* objectA, GCObject* objectB, bool linked)
	{
		GraphModification graphMod;
		graphMod.objectA = objectA;
		graphMod.objectB = objectB;
		graphMod.type = linked ? GraphModification::ADD_EDGE : GraphModification::DEL_EDGE;
		this->graphModQueue->enqueue(graphMod);
	}

	bool GarbageCollector::Startup(void)
	{
		if (this->threadHandle != nullptr)
			return false;

		this->threadExitSignaled = false;
		this->threadHandle = ::CreateThread(nullptr, 0, &GarbageCollector::ThreadEntryPoint, this, 0, nullptr);
		return true;
	}

	bool GarbageCollector::Shutdown(void)
	{
		if (this->threadHandle == nullptr)
			return false;

		this->threadExitSignaled = true;
		::WaitForSingleObject(this->threadHandle, INFINITE);

		this->FreeObjects();

		return true;
	}

	void GarbageCollector::StallUntilCaughtUp(void)
	{
		// We might need a mutex to protect access to the semaphore, but I'm just going to leave this as is for now.
		this->caughtUpSemaphore = ::CreateSemaphore(nullptr, 0, 1, nullptr);
		::WaitForSingleObject(this->caughtUpSemaphore, INFINITE);
		::CloseHandle(this->caughtUpSemaphore);
		this->caughtUpSemaphore = nullptr;
	}

	uint32_t GarbageCollector::ObjectCount(void)
	{
		return this->objectList.GetCount();
	}

	/*static*/ unsigned long __stdcall GarbageCollector::ThreadEntryPoint(LPVOID param)
	{
		GC()->Run();
		return 0;
	}

	void GarbageCollector::Run(void)
	{
		while (!this->threadExitSignaled)
		{
			if (this->UpdateGraph())
			{
				// If the graph changes, all bets are off.  Reset our counts.
				this->workCount = 0;
				this->targetWorkCount = this->objectList.GetCount();
			}

			if (this->objectList.GetCount() > 0)
			{
				GCObject* rootObject = this->objectList.GetHead()->value;
				LinkedList<GCObject*> spanningTreeList;
				
				// In addition to finding the spanning tree containing the given object,
				// this will also remove the spanning tree members from our list of objects.
				this->FindSpanningTree(rootObject, spanningTreeList, true);

				// Collectable or not, bump our count telling us how far away we are from being caught up.
				this->workCount += spanningTreeList.GetCount();

				// Are the objects ready to be garbage collected?
				if (this->CanCollectAll(spanningTreeList))
				{
					// Yes!  Throw them on the garbage collection queue to be deleted on the main thread.
					for (LinkedList<GCObject*>::Node* node = spanningTreeList.GetHead(); node; node = node->GetNext())
					{
						GCObject* object = node->value;
						this->garbageQueue->enqueue(object);	// This is lock-free.
					}
				}
				else
				{
					// No!  Add them back to our list.  Note, however, that we add the objects to the end of
					// our list so that we're less likely to reconsider them any time soon.
					for (LinkedList<GCObject*>::Node* node = spanningTreeList.GetHead(); node; node = node->GetNext())
					{
						GCObject* object = node->value;
						this->objectList.AddTail(object);
						object->node = this->objectList.GetTail();
					}
				}
			}

			// If we're caught up, signal the main thread if it cares.
			if (this->workCount >= this->targetWorkCount && this->caughtUpSemaphore != nullptr)
				::ReleaseSemaphore(this->caughtUpSemaphore, 1, nullptr);
		}

		this->UpdateGraph();

		// If we're going down, then it should be safe to unconditionally dump everything onto the garbage collection queue.
		while (this->objectList.GetCount() > 0)
		{
			LinkedList<GCObject*>::Node* node = this->objectList.GetHead();
			GCObject* object = node->value;
			this->objectList.Remove(node);
			this->garbageQueue->enqueue(object);
		}
	}

	void GarbageCollector::FindSpanningTree(GCObject* rootObject, LinkedList<GCObject*>& spanningTreeList, bool removeFromObjectList)
	{
		this->spanningTreeKey++;

		LinkedList<GCObject*> objectQueue;
		objectQueue.AddTail(rootObject);
		rootObject->spanningTreeKey = this->spanningTreeKey;

		while (objectQueue.GetCount() > 0)
		{
			LinkedList<GCObject*>::Node* node = objectQueue.GetHead();
			GCObject* object = node->value;
			objectQueue.Remove(node);

			spanningTreeList.AddTail(object);

			if (removeFromObjectList)
			{
				assert(object->node != nullptr);
				this->objectList.Remove(object->node);
				object->node = nullptr;
			}

			for (GCObject* adjacentObject : *object->adjacencySet)
			{
				if (adjacentObject->spanningTreeKey != this->spanningTreeKey)
				{
					objectQueue.AddTail(adjacentObject);
					adjacentObject->spanningTreeKey = this->spanningTreeKey;
				}
			}
		}
	}

	bool GarbageCollector::CanCollectAll(LinkedList<GCObject*>& objectList)
	{
		for (LinkedList<GCObject*>::Node* node = objectList.GetHead(); node; node = node->GetNext())
		{
			GCObject* object = node->value;
			if (object->ReturnType() == GCObject::ANCHOR || !object->marked)
				return false;
		}

		return true;
	}

	bool GarbageCollector::UpdateGraph(void)
	{
		bool graphUpdated = false;
		GraphModification graphMod;
		while (this->graphModQueue->try_dequeue(graphMod))
		{
			graphUpdated = true;
			switch (graphMod.type)
			{
				case GraphModification::ADD_VERTEX:
				{
					this->objectList.AddHead(graphMod.objectA);
					graphMod.objectA->node = this->objectList.GetHead();
					break;
				}
				case GraphModification::ADD_EDGE:
				{
					LinkedList<GCObject*> spanningTreeList;

					if (!graphMod.objectA->marked && graphMod.objectB->marked)
						this->FindSpanningTree(graphMod.objectA, spanningTreeList, false);
					else if (graphMod.objectA->marked && !graphMod.objectB->marked)
						this->FindSpanningTree(graphMod.objectB, spanningTreeList, false);

					for (LinkedList<GCObject*>::Node* node = spanningTreeList.GetHead(); node; node = node->GetNext())
					{
						GCObject* object = node->value;
						object->marked = true;
					}

					graphMod.objectA->adjacencySet->insert(graphMod.objectB);
					graphMod.objectB->adjacencySet->insert(graphMod.objectA);
					break;
				}
				case GraphModification::DEL_EDGE:
				{
					graphMod.objectA->adjacencySet->erase(graphMod.objectB);
					graphMod.objectB->adjacencySet->erase(graphMod.objectA);
					break;
				}
				case GraphModification::DEL_VERTEX:
				{
					this->objectList.RemoveNode(graphMod.objectA->node);
					graphMod.objectA->node = nullptr;
					for (GCObject* adjacentObject : *graphMod.objectA->adjacencySet)
						adjacentObject->adjacencySet->erase(graphMod.objectA);
					graphMod.objectA->adjacencySet->clear();
					assert(graphMod.objectA->ReturnType() == GCObject::ANCHOR);
					this->garbageQueue->enqueue(graphMod.objectA);
					break;
				}
			}
		}

		return graphUpdated;
	}

	void GarbageCollector::FreeObjects(void)
	{
		GCObject* object = nullptr;
		while (this->garbageQueue->try_dequeue(object))
			delete object;
	}
}