#pragma once

#include <string>
#include <filesystem>

namespace Powder
{
	class Error;

	class PathResolver
	{
	public:
		PathResolver();
		virtual ~PathResolver();

		enum
		{
			SEARCH_CWD = 1,
			SEARCH_BASE = 2
		};

		std::string ResolvePath(const std::string& givenPath, int searchFlags, Error& error);
		bool FindBaseDirectoryUsingModulePath();
		bool SearchDirectoryForFile(const std::filesystem::path& searchDirectory, const std::filesystem::path& relativePath, std::filesystem::path& absolutePath);

		std::filesystem::path baseDirectory;
	};

	extern PathResolver pathResolver;
}