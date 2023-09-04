#pragma once

#include "GCCollectable.h"
#include "JsonValue.h"
#include <string>
#include <cstdint>

namespace Powder
{
	class Executable : public GCCollectable
	{
	public:
		Executable();
		virtual ~Executable();

		void Save(const std::string& byteCodeFile) const;
		void Load(const std::string& byteCodeFile);

		void Clear();

		uint8_t* byteCodeBuffer;
		uint64_t byteCodeBufferSize;

		ParseParty::JsonObject* debugInfoDoc;
	};
}