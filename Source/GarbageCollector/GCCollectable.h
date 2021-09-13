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

		void IncRef(uint32_t count = 1);
		void DecRef(uint32_t count = 1);

	private:
		uint32_t refCount;
		uint32_t spanningTreeKey;
		std::set<GCCollectable*>* adjacencySet;
		LinkedList<GCCollectable*>::Node* node;
	};
}