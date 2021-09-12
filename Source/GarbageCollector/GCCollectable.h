#pragma once

#include "Defines.h"
#include "LinkedList.hpp"
#include <stdint.h>
#include <set>

namespace Powder
{
	// Half the scope of these class instances is managed by the GC system.
	// Feel free to allocate them yourself, but do not free them yourself.
	class POWDER_API GCCollectable
	{
		friend class GarbageCollector;

	public:
		GCCollectable();
		virtual ~GCCollectable();

		void ConnectTo(GCCollectable* collectable);
		void DisconnectFrom(GCCollectable* collectable);

		void AddRef();
		void DecRef();

	private:

		uint32_t refCount;
		uint32_t spanningTreeKey;
		bool armedForDelete;
		std::set<GCCollectable*>* adjacencySet;
		LinkedList<GCCollectable*>::Node* node;
	};
}