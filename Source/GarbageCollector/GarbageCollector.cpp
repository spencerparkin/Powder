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
		this->graphModQueue = new moodycamel::ConcurrentQueue<GraphModification>();
		this->garbageQueue = new moodycamel::ConcurrentQueue<GCCollectable*>();
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

	void GarbageCollector::TrackCollectable(GCCollectable* collectable)
	{
		GraphModification graphMod;
		graphMod.collectableA = collectable;
		graphMod.collectableB = nullptr;
		graphMod.type = GraphModification::ADD_VERTEX;
		this->graphModQueue->enqueue(graphMod);
	}

	void GarbageCollector::RelateCollectables(GCCollectable* collectableA, GCCollectable* collectableB, bool linked)
	{
		GraphModification graphMod;
		graphMod.collectableA = collectableA;
		graphMod.collectableB = collectableB;
		graphMod.type = linked ? GraphModification::ADD_EDGE : GraphModification::DELETE_EDGE;
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
	}

	uint32_t GarbageCollector::TrackingCount(void)
	{
		return this->collectableList.GetCount();
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

			GCCollectable* rootCollectable = this->FindUnvisitedCollectable();
			if (!rootCollectable)
				this->spanningTreeKey++;
			else
			{
				LinkedList<GCCollectable*> spanningTreeList;
				
				// In addition to update the spanning tree key of all found collectables,
				// this will also remove them from our list of collectables.
				this->FindSpanningTree(rootCollectable, spanningTreeList);

				// Are the collectables ready to be garbage collected?
				if (this->CanCollectAll(spanningTreeList))
				{
					// Yes!  Throw them on the garbage collection queue to be deleted on the main thread.
					for (LinkedList<GCCollectable*>::Node* node = spanningTreeList.GetHead(); node; node = node->GetNext())
					{
						GCCollectable* collectable = node->value;
						this->garbageQueue->enqueue(collectable);
					}
				}
				else
				{
					// No!  Add them back to our list.  Note, however, that we add the collectables back at the end of
					// our list so that we're less likely to reconsider them any time soon.
					for (LinkedList<GCCollectable*>::Node* node = spanningTreeList.GetHead(); node; node = node->GetNext())
					{
						GCCollectable* collectable = node->value;
						this->collectableList.AddTail(collectable);
						collectable->node = this->collectableList.GetTail();
					}
				}
			}
		}

		this->UpdateGraph();

		// If we're going down, then it should be safe to unconditionally dump everything onto the garbage collection queue.
		while (this->collectableList.GetCount() > 0)
		{
			LinkedList<GCCollectable*>::Node* node = this->collectableList.GetHead();
			GCCollectable* collectable = node->value;
			this->collectableList.Remove(node);
			this->garbageQueue->enqueue(collectable);
		}
	}

	void GarbageCollector::FindSpanningTree(GCCollectable* rootCollectable, LinkedList<GCCollectable*>& spanningTreeList)
	{
		LinkedList<GCCollectable*> collectableQueue;
		collectableQueue.AddTail(rootCollectable);
		rootCollectable->spanningTreeKey = this->spanningTreeKey;

		while (collectableQueue.GetCount() > 0)
		{
			LinkedList<GCCollectable*>::Node* node = collectableQueue.GetHead();
			GCCollectable* collectable = node->value;
			collectableQueue.Remove(node);

			spanningTreeList.AddTail(collectable);
			this->collectableList.Remove(collectable->node);
			collectable->node = nullptr;

			for (GCCollectable* adjacentCollectable : *collectable->adjacencySet)
			{
				if (adjacentCollectable->spanningTreeKey != this->spanningTreeKey)
				{
					collectableQueue.AddTail(adjacentCollectable);
					adjacentCollectable->spanningTreeKey = this->spanningTreeKey;
				}
			}
		}
	}

	bool GarbageCollector::CanCollectAll(LinkedList<GCCollectable*>& collectableList)
	{
		for (LinkedList<GCCollectable*>::Node* node = collectableList.GetHead(); node; node = node->GetNext())
		{
			GCCollectable* collectable = node->value;
			if (collectable->refCount > 0)
				return false;
		}

		return true;
	}

	GCCollectable* GarbageCollector::FindUnvisitedCollectable(void)
	{
		// Note that since newly tracked collectables are always added to the front of
		// our list, here we're generally searching from newest to oldest.
		for (LinkedList<GCCollectable*>::Node* node = collectableList.GetHead(); node; node = node->GetNext())
		{
			GCCollectable* collectable = node->value;
			if (collectable->spanningTreeKey != this->spanningTreeKey)
				return collectable;
		}

		return nullptr;
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
					this->collectableList.AddHead(graphMod.collectableA);
					graphMod.collectableA->node = this->collectableList.GetHead();
					break;
				}
				case GraphModification::ADD_EDGE:
				{
					graphMod.collectableA->adjacencySet->insert(graphMod.collectableB);
					graphMod.collectableB->adjacencySet->insert(graphMod.collectableA);
					break;
				}
				case GraphModification::DELETE_EDGE:
				{
					graphMod.collectableA->adjacencySet->erase(graphMod.collectableB);
					graphMod.collectableB->adjacencySet->erase(graphMod.collectableA);
					break;
				}
			}
		}
	}

	void GarbageCollector::FreeObjects(void)
	{
		GCCollectable* collectable = nullptr;
		while (this->garbageQueue->try_dequeue(collectable))
			delete collectable;
	}
}