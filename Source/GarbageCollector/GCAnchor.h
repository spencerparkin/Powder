#pragma once

#include "GCObject.h"

namespace Powder
{
	class GCAnchor : public GCObject
	{
	public:
		GCAnchor();
		virtual ~GCAnchor();

		virtual Type ReturnType() const override;
	};
}