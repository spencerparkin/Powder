#include "GCCollectable.h"
#include "GarbageCollector.h"

namespace Powder
{
	GCCollectable::GCCollectable()
	{
		this->armedForDelete = false;
	}

	/*virtual*/ GCCollectable::~GCCollectable()
	{
	}

	/*virtual*/ bool GCCollectable::CanBeCollected() const
	{
		return this->armedForDelete;
	}

	/*virtual*/ bool GCCollectable::IsAnchor() const
	{
		return false;
	}

	/*virtual*/ void GCCollectable::PossiblyArmForDelete(GCObject* adjacentObject)
	{
		if (adjacentObject->CanBeCollected() || adjacentObject->IsAnchor())
			this->armedForDelete = true;
	}
}