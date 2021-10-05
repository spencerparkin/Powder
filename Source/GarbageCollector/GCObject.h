#pragma once

#include "Defines.h"
#include "LinkedList.hpp"
#include <stdint.h>
#include <map>

namespace Powder
{
	class POWDER_API GCObject
	{
		friend class GarbageCollector;

	public:
		GCObject();
		virtual ~GCObject();

		enum Type
		{
			ANCHOR,
			COLLECTABLE
		};

		virtual Type ReturnType() const = 0;

		void ConnectTo(GCObject* object);
		void DisconnectFrom(GCObject* object);

	protected:

		void AddAdjacency(GCObject* object);
		bool RemoveAdjacency(GCObject* object);

		uint32_t spanningTreeKey;
		std::map<GCObject*, uint32_t>* adjacencyMap;
		LinkedList<GCObject*>::Node* node;

		// We maintain the property that if any node is (un)marked, then all
		// nodes connected to it, directly or indirectly, are also (un)marked.
		bool marked;

#if defined GC_DEBUG
		// Rather than actually delete the object, we can mark it as deleted
		// if running the GC in a debug mode.
		bool deleted;
#endif
	};
}