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

	/*virtual*/ bool GCCollectable::IsReference(void)
	{
		return false;
	}
}