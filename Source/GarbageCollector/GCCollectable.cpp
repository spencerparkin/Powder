#include "GCCollectable.h"
#include "GarbageCollector.h"

namespace Powder
{
	GCCollectable::GCCollectable()
	{
	}

	/*virtual*/ GCCollectable::~GCCollectable()
	{
	}

	/*virtual*/ GCObject::Type GCCollectable::ReturnType() const
	{
		return COLLECTABLE;
	}
}