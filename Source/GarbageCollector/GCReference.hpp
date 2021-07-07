#pragma once

#include "Defines.h"
#include "GCObject.h"

namespace Powder
{
	// Memory for instances of this class are not freed by the GC.
	// Rather, the calling program is responsible for managing their scope.
	// The purpose of this class in the GC is to keep GCCollectable class
	// instances in memory for as long as the reference object references
	// them, directly or indirectly.
	template<typename T>
	class POWDER_API GCReference : public GCObject
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
				this->pointer->ConnectTo(this);
		}

		GCReference(const GCReference<T>& reference)
		{
			this->pointer = reference.pointer;

			if (this->pointer)
				this->pointer->ConnectTo(this);
		}

		virtual ~GCReference()
		{
			if (this->pointer)
				this->pointer->DisconnectFrom(this);
		}

		virtual bool IsReference(void) override
		{
			return true;
		}

		void operator=(const GCReference& reference)
		{
			if (this->pointer)
				this->pointer->DisconnectFrom(this);

			this->pointer = reference.pointer;

			if (this->pointer)
				this->pointer->ConnectTo(this);
		}

		void operator=(T* pointer)
		{
			if (this->pointer)
				this->pointer->DisconnectFrom(this);

			this->pointer = pointer;

			if (this->pointer)
				this->pointer->ConnectTo(this);
		}

		void Clear(void)
		{
			if (this->pointer)
				this->pointer->DisconnectFrom(this);

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