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

	/*virtual*/ bool GCAnchor::CanBeArmedForDelete() const
	{
		return false;
	}

	/*virtual*/ void GCAnchor::ArmIfPossible()
	{
		// Do nothing.
	}
}