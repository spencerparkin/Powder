#pragma once

#include "Defines.h"
#include <string>
#include <vector>

namespace Powder
{
	class POWDER_API Error
	{
	public:
		Error();
		virtual ~Error();

		operator bool() const;
		operator std::string() const;

		void Clear();
		void Add(const std::string& errorMessage);

	private:

		std::vector<std::string>* errorMessageArray;
	};
}