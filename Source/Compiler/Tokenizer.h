#pragma once

#include "Defines.h"
#include <cstdint>
#include "LinkedList.hpp"
#include "Exceptions.hpp"
#include <string>
#include <vector>

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

		static void Replace(std::string& string, const std::string& oldSubString, const std::string& newSubString);
		static bool IsAnyChar(char givenChar, const char* charSet);

	private:
		Token GenerateToken(const char* programCodeBuffer, uint64_t& programCodeBufferLocation, FileLocation& fileLocation);

		std::string SoakUpToken(const char* programCodeBuffer, uint64_t& programCodeBufferLocation, const std::vector<std::string>& optionsArray);

		std::vector<std::string>* operatorArray;
	};
}