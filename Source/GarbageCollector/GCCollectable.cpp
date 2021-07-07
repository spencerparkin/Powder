#include "GCCollectable.h"
#include "GarbageCollector.h"

namespace Powder
{
	GCCollectable::GCCollectable()
	{
		GarbageCollector::GC()->honestCollectableCount++;
	}

	/*virtual*/ GCCollectable::~GCCollectable()
	{
		GarbageCollector::GC()->honestCollectableCount--;
	}

	/*virtual*/ bool GCCollectable::IsReference(void)
	{
		return false;
	}
}