#pragma once

#include "GCCollectable.h"
#include <string>
#include <cstdint>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

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

		rapidjson::Document* debugInfoDoc;
	};
}