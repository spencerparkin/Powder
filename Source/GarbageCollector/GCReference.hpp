#pragma once

#include "Defines.h"

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
		}

		GCReference(T* pointer)
		{
			this->pointer = pointer;

			if (this->pointer)
				this->pointer->IncRef();
		}

		GCReference(const GCReference<T>& reference)
		{
			this->pointer = reference.pointer;

			if (this->pointer)
				this->pointer->IncRef();
		}

		virtual ~GCReference()
		{
			if (this->pointer)
				this->pointer->DecRef();
		}

		void operator=(const GCReference& reference)
		{
			if (this->pointer)
				this->pointer->DecRef();

			this->pointer = reference.pointer;

			if (this->pointer)
				this->pointer->IncRef();
		}

		void operator=(T* pointer)
		{
			if (this->pointer)
				this->pointer->DecRef();

			this->pointer = pointer;

			if (this->pointer)
				this->pointer->IncRef();
		}

		void Clear(void)
		{
			if (this->pointer)
				this->pointer->DecRef();

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
	};
}