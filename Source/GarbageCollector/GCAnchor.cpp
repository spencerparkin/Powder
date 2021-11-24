#include "GCAnchor.h"

namespace Powder
{
	GCAnchor::GCAnchor()
	{
		this->marked = true;
	}

	/*virtual*/ GCAnchor::~GCAnchor()
	{
	}

	/*virtual*/ GCObject::Type GCAnchor::ReturnType() const
	{
		return ANCHOR;
	}
}