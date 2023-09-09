#pragma once

#include "Collectable.h"
#include "JsonValue.h"
#include <string>
#include <cstdint>

namespace Powder
{
	class Error;

	class Executable : public GC::Collectable
	{
	public:
		Executable();
		virtual ~Executable();

		bool Save(const std::string& byteCodeFile, Error& error) const;
		bool Load(const std::string& byteCodeFile, Error& error);

		void Clear();

		uint8_t* byteCodeBuffer;
		uint64_t byteCodeBufferSize;

		ParseParty::JsonObject* debugInfoDoc;
	};
}