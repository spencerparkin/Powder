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

	/*virtual*/ bool GCCollectable::CanBeArmedForDelete() const
	{
		return !this->armedForDelete;
	}

	/*virtual*/ void GCCollectable::ArmIfPossible()
	{
		this->armedForDelete = true;
	}
}