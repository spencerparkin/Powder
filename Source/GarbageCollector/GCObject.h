#pragma once

#include "Defines.h"
#include <set>

namespace Powder
{
	class POWDER_API GCObject
	{
		friend class GarbageCollector;

	public:
		GCObject();
		virtual ~GCObject();

		virtual bool IsReference(void) = 0;

		void ConnectTo(GCObject* object);
		void DisconnectFrom(GCObject* object);

	protected:

		std::set<GCObject*>* adjacencies;
		uint32_t visitNumber;
	};
}