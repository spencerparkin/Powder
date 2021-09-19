#pragma once

#include "Defines.h"
#include "LinkedList.hpp"
#include <stdint.h>
#include <set>

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
		uint32_t spanningTreeKey;
		std::set<GCObject*>* adjacencySet;
		LinkedList<GCObject*>::Node* node;

		// We maintain the property that if any node is (un)marked, then all
		// nodes connected to it, directly or indirectly, are also (un)marked.
		bool marked;
	};
}