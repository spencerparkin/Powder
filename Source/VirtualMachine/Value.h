#pragma once

#include "GCCollectable.h"

namespace Powder
{
	// These are the fundamental units of data processed by the machine.
	// A value may refer to a mutable or immutable type of data.
	class POWDER_API Value : public GCCollectable
	{
	public:

		Value();
		virtual ~Value();

		virtual Value* Copy() const;
	};
}