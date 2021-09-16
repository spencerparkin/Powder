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

	/*virtual*/ void GCAnchor::ArmForDelete()
	{
		// We do nothing here.
	}
}