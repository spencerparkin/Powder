#pragma once

#include "Defines.h"
#include "GCAnchor.h"
#include "GarbageCollector.h"

namespace Powder
{
	// The scope of these objects is managed by the user, not the GC system.
	// Furthermore, no GCCollectable derivative should ever own an instance of this class.
	// Rather, such a class should own a GCSteward class instance.
	template<typename T>
	class POWDER_API GCReference
	{
	public:
		GCReference()
		{
			this->pointer = nullptr;
			this->anchor = new GCAnchor();
		}

		GCReference(T* pointer)
		{
			this->pointer = pointer;
			this->anchor = new GCAnchor();

			if (this->pointer)
				this->pointer->ConnectTo(this->anchor);
		}

		GCReference(const GCReference<T>& reference)
		{
			this->pointer = reference.pointer;
			this->anchor = new GCAnchor();

			if (this->pointer)
				this->pointer->ConnectTo(this->anchor);
		}

		virtual ~GCReference()
		{
			GarbageCollector::GC()->RemoveObject(this->anchor);
		}

		void operator=(const GCReference& reference)
		{
			if (this->pointer)
				this->pointer->DisconnectFrom(this->anchor);

			this->pointer = reference.pointer;

			if (this->pointer)
				this->pointer->ConnectTo(this->anchor);
		}

		void operator=(T* pointer)
		{
			if (this->pointer)
				this->pointer->DisconnectFrom(this->anchor);

			this->pointer = pointer;

			if (this->pointer)
				this->pointer->ConnectTo(this->anchor);
		}

		void Clear(void)
		{
			if (this->pointer)
				this->pointer->DisconnectFrom(this->anchor);

			this->pointer = nullptr;
		}

		T* operator->()
		{
			return pointer;
		}

		const T* operator->() const
		{
			return pointer;
		}

		operator T*()
		{
			return pointer;
		}

		operator T*() const
		{
			return pointer;
		}

		T* Ptr()
		{
			return pointer;
		}

		const T* Ptr() const
		{
			return pointer;
		}

	private:

		T* pointer;
		GCAnchor* anchor;
	};
}