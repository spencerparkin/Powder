#include "GCAnchor.h"

namespace Powder
{
	GCAnchor::GCAnchor()
	{
	}

	/*virtual*/ GCAnchor::~GCAnchor()
	{
	}

	/*virtual*/ bool GCAnchor::CanBeCollected() const
	{
		return false;
	}

	/*virtual*/ bool GCAnchor::IsAnchor() const
	{
		return true;
	}

	/*virtual*/ void GCAnchor::PossiblyArmForDelete(GCObject* adjacentObject)
	{
		// We do nothing here.
	}
}