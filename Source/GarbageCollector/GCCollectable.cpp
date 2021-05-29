#include "GCCollectable.h"

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