#pragma once

#include "Defines.h"

namespace Powder
{
	class POWDER_API RecursionGuard
	{
	public:
		RecursionGuard(bool* recursionFlag)
		{
			this->recursionFlag = recursionFlag;
			if (*recursionFlag)
				this->recursing = true;
			else
			{
				this->recursing = false;
				*recursionFlag = true;
			}
		}

		virtual ~RecursionGuard()
		{
			*this->recursionFlag = false;
		}

		bool IsRecursing() const
		{
			return this->recursing;
		}

	private:
		
		bool* recursionFlag;
		bool recursing;
	};
}