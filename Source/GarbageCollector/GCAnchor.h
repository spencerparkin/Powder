#pragma once

#include "GCObject.h"

namespace Powder
{
	class GCAnchor : public GCObject
	{
	public:
		GCAnchor();
		virtual ~GCAnchor();

		virtual bool CanBeCollected() const override;
		virtual bool IsAnchor() const override;
		virtual void PossiblyArmForDelete(GCObject* adjacentObject) override;
	};
}