#pragma once

#include <cstdint>

namespace Powder
{
	class Stream
	{
	public:
		Stream();
		virtual Stream();

		virtual uint64_t Write(const uint8_t* buffer, uint64_t bufferSize) = 0;
		virtual uint64_t Read(uint8_t* buffer, uint64_t bufferSize) = 0;
	};

	class MemoryStream : public Stream
	{
	public:
	};
}