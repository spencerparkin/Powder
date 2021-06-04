#pragma once

#include "Defines.h"

namespace Powder
{
	class GCCollectable;

	template<typename T>
	class POWDER_API GCSteward
	{
	public:
		GCSteward(GCCollectable* owner)
		{
			this->owner = owner;
			this->object = nullptr;
		}

		virtual ~GCSteward()
		{
		}

		T* Get()
		{
			return this->object;
		}

		void Set(T* object)
		{
			if (this->object)
				this->owner->DisownObject(this->object);

			this->object = object;

			if (this->object)
				this->owner->OwnObject(this->object);
		}

	private:

		T* object;
		GCCollectable* owner;
	};
}