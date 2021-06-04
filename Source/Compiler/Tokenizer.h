#pragma once

#include "Defines.h"
#include <cstdint>
#include "LinkedList.hpp"
#include "Exceptions.hpp"
#include <string>

namespace Powder
{
	struct Token
	{
		enum Type
		{
			UNKNOWN,
			IDENTIFIER,
			NUMBER,
			STRING,
			OPERATOR,
			DELIMETER,
			OPENER,
			CLOSER
		};

		Token()
		{
			this->type = UNKNOWN;
			this->fileLocation.lineNumber = -1;
			this->fileLocation.columnNumber = -1;
		}

		std::string text;
		Type type;
		FileLocation fileLocation;
	};

	typedef LinkedList<Token> TokenList;

	class POWDER_API Tokenizer
	{
	public:
		Tokenizer();
		virtual ~Tokenizer();

		void Tokenize(const char* programCodeBuffer, TokenList& tokenList);

	private:
		Token GenerateToken(const char* programCodeBuffer, uint64_t& programCodeBufferLocation, FileLocation& fileLocation);

		bool IsAnyChar(char givenChar, const char* charSet);
	};
}