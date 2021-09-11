#pragma once

#include "Defines.h"

namespace Powder
{
	class POWDER_API GCObject
	{
	public:
		GCObject();
		virtual ~GCObject();

		virtual bool IsReference(void) = 0;

		void ConnectTo(GCObject* object);
		void DisconnectFrom(GCObject* object);
	};
}