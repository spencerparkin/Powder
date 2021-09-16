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

		virtual bool CanBeCollected() const = 0;
		virtual bool IsAnchor() const = 0;
		virtual void PossiblyArmForDelete(GCObject* adjacentObject) = 0;

		void ConnectTo(GCObject* object);
		void DisconnectFrom(GCObject* object);

	private:
		uint32_t spanningTreeKey;
		std::set<GCObject*>* adjacencySet;
		LinkedList<GCObject*>::Node* node;
	};
}