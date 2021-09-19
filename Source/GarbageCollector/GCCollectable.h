#pragma once

#include "Defines.h"
#include "GCObject.h"

namespace Powder
{
	// Half the scope of these class instances is managed by the GC system.
	// Feel free to allocate them yourself, but do not free them yourself.
	class POWDER_API GCCollectable : public GCObject
	{
	public:
		GCCollectable();
		virtual ~GCCollectable();

		virtual Type ReturnType() const override;
	};
}