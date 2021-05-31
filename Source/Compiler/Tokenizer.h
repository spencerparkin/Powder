#pragma once

#include <cstdint>
#include "LinkedList.hpp"
#include <string>

namespace Powder
{
	struct Token
	{
		enum Type
		{
			IDENTIFIER,
			BINARY_OPERATOR,
			UNARY_OPERATOR,
			DELIMETER,
			OPEN_PARAN,
			CLOSE_PARAN,
			OPEN_CURLY,
			CLOSE_CURLY
		};

		std::string text;
		Type type;
		uint32_t lineNumber;
	};

	typedef LinkedList<Token> TokenList;

	class Tokenizer
	{
	public:
		Tokenizer();
		virtual ~Tokenizer();

		void Tokenize(const char* programCodeBuffer, TokenList& tokenList);
	};
}