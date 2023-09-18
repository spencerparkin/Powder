#include "PathResolver.h"
#include "ModuleMain.h"
#include "Error.h"
#include <Windows.h>

namespace Powder
{
	PathResolver pathResolver;

	PathResolver::PathResolver()
	{
		this->baseDirectory = new std::filesystem::path;
	}

	/*virtual*/ PathResolver::~PathResolver()
	{
		delete this->baseDirectory;
	}

	std::string PathResolver::ResolvePath(const std::string& givenPath, int searchFlags, Error& error)
	{
		std::filesystem::path unresolvedPath = givenPath;
		std::filesystem::path resolvedPath;

		if (unresolvedPath.is_absolute())
			resolvedPath = unresolvedPath;
		else if (unresolvedPath.is_relative())
		{
			if ((searchFlags & SEARCH_CWD) != 0)
			{
				resolvedPath = std::filesystem::current_path() / unresolvedPath;
				if (!std::filesystem::exists(resolvedPath))
					this->SearchDirectoryForFile(std::filesystem::current_path(), unresolvedPath, resolvedPath);
			}
			
			if (!std::filesystem::exists(resolvedPath))
			{
				if (*this->baseDirectory == "")
					this->FindBaseDirectoryUsingModulePath();

				if ((searchFlags & SEARCH_BASE) != 0)
				{
					resolvedPath = *this->baseDirectory / unresolvedPath;
					if (!std::filesystem::exists(resolvedPath))
						this->SearchDirectoryForFile(*this->baseDirectory, unresolvedPath, resolvedPath);
				}
			}
		}

		if (!std::filesystem::exists(resolvedPath))
		{
			error.Add(std::format("Could not find file: {}", givenPath.c_str()));
			return "";
		}

		return resolvedPath.string();
	}

	bool PathResolver::SearchDirectoryForFile(const std::filesystem::path& searchDirectory, const std::filesystem::path& relativePath, std::filesystem::path& absolutePath)
	{
		for (std::filesystem::directory_entry dir_entry : std::filesystem::recursive_directory_iterator(searchDirectory))
		{
			if (dir_entry.is_directory())
			{
				absolutePath = dir_entry.path() / relativePath;
				if (std::filesystem::exists(absolutePath))
					return true;
			}
		}

		return false;
	}

	bool PathResolver::FindBaseDirectoryUsingModulePath()
	{
		char modulePathBuffer[1024];
		DWORD modulePathBufferSize = sizeof(modulePathBuffer);
		::GetModuleFileNameA((HMODULE)moduleHandle, modulePathBuffer, modulePathBufferSize);

		std::string modulePath = modulePathBuffer;
		std::filesystem::path directory = std::filesystem::path(modulePath);

		while (directory.has_filename())
		{
			if (directory.filename() == "Powder")
			{
				*this->baseDirectory = directory;
				return true;
			}

			directory = directory.parent_path();
		}

		return false;
	}
}