#pragma once

#include "Defines.h"
#include <string>
#include <filesystem>

namespace Powder
{
	class Error;

	class POWDER_API PathResolver
	{
	public:
		PathResolver();
		virtual ~PathResolver();

		enum
		{
			SEARCH_CWD = 1,
			SEARCH_BASE = 2,
			SEARCH_MODULES = 4
		};

		std::string ResolvePath(const std::string& givenPath, int searchFlags, Error& error);
		bool FindBaseDirectoryUsingModulePath();
		bool SearchDirectoryForFile(const std::filesystem::path& searchDirectory, const std::filesystem::path& relativePath, std::filesystem::path& absolutePath);

		std::filesystem::path* baseDirectory;
	};

	POWDER_API extern PathResolver pathResolver;
}