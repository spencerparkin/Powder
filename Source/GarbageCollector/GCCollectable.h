#pragma once

#include "Defines.h"
#include "GCObject.h"
#include "GCReference.hpp"

namespace Powder
{
	// Memory for instances of this class are freed by the GC if
	// it finds that the object is not connected to a GCReference
	// object, directly or indirectly.  Being referenced in a way
	// that retains the memory is a transitive property.
	class POWDER_API GCCollectable : public GCObject
	{
	public:
		GCCollectable();
		virtual ~GCCollectable();

		virtual bool IsReference(void) override;
	};
}