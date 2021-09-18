#include "GarbageCollector.h"
#include "GCCollectable.h"

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
		// TODO: Block on semaphore here.  Semaphore is released by the GC thread when it knows it's caught up.
		//       Note that until this is implemented, the IOTest.pow script will crash.
	}

	uint32_t GarbageCollector::ObjectCount(void)
	{
		return this->objectList.GetCount();
	}

	/*static*/ DWORD __stdcall GarbageCollector::ThreadEntryPoint(LPVOID param)
	{
		GC()->Run();
		return 0;
	}

	void GarbageCollector::Run(void)
	{
		while (!this->threadExitSignaled)
		{
			this->UpdateGraph();

			if (this->objectList.GetCount() > 0)
			{
				GCObject* rootObject = this->objectList.GetHead()->value;
				LinkedList<GCObject*> spanningTreeList;
				
				// In addition to finding the spanning tree containing the given object,
				// this will also remove the spanning tree members from our list of objects.
				this->FindSpanningTree(rootObject, spanningTreeList, true);

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
			if (!object->CanBeCollected())
				return false;
		}

		return true;
	}

	void GarbageCollector::UpdateGraph(void)
	{
		GraphModification graphMod;
		while (this->graphModQueue->try_dequeue(graphMod))
		{
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
					graphMod.objectA->adjacencySet->insert(graphMod.objectB);
					graphMod.objectB->adjacencySet->insert(graphMod.objectA);

					if (graphMod.objectA->CanBeArmedForDelete() || graphMod.objectB->CanBeArmedForDelete())
					{
						LinkedList<GCObject*> spanningTreeList;
						this->FindSpanningTree(graphMod.objectA, spanningTreeList, false);
						for (LinkedList<GCObject*>::Node* node = spanningTreeList.GetHead(); node; node = node->GetNext())
						{
							GCObject* object = node->value;
							object->ArmIfPossible();
						}
					}

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
					for (GCObject* adjacentObject : *graphMod.objectA->adjacencySet)
						adjacentObject->adjacencySet->erase(graphMod.objectA);
					graphMod.objectA->adjacencySet->clear();
					this->garbageQueue->enqueue(graphMod.objectA);
					break;
				}
			}
		}
	}

	void GarbageCollector::FreeObjects(void)
	{
		GCObject* object = nullptr;
		while (this->garbageQueue->try_dequeue(object))
			delete object;
	}
}