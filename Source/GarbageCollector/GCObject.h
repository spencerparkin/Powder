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

		static void* Allocate(uint64_t size);
		static void Deallocate(void* memory);

		// This is used primarily by extension modules to prevent cross heap interaction.
		// Specifically, we don't want memory allocated in an extension module's heap to
		// get freed by us in our heap.  That works, provided both are using the same
		// runtime, but it is not always guarenteed.
		template<typename T>
		static T* Create()
		{
			void* memory = Allocate(sizeof(T));
			T* value = new (memory) T();
			value->placementNewUsed = true;
			return value;
		}

	protected:

		std::set<GCObject*>* adjacencies;
		uint32_t visitNumber;
		bool placementNewUsed;
	};
}