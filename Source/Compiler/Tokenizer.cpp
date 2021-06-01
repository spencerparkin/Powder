#include "Tokenizer.h"
#include <ctype.h>
#include <cstdint>
#include "Exceptions.hpp"

namespace Powder
{
	Tokenizer::Tokenizer()
	{
	}

	/*virtual*/ Tokenizer::~Tokenizer()
	{
	}

	void Tokenizer::Tokenize(const char* programCodeBuffer, TokenList& tokenList)
	{
		uint64_t programCodeBufferLocation = 0;
		uint16_t lineNumber = 1;
		while (programCodeBuffer[programCodeBufferLocation] != '\0')
		{
			Token token = this->GenerateToken(programCodeBuffer, programCodeBufferLocation, lineNumber);
			if (token.type != Token::UNKNOWN)
				tokenList.AddTail(token);
		}
	}

	Token Tokenizer::GenerateToken(const char* programCodeBuffer, uint64_t& programCodeBufferLocation, uint16_t& lineNumber)
	{
		Token token;
		token.lineNumber = lineNumber;
		uint64_t i = programCodeBufferLocation;

		if (programCodeBuffer[i] == '\n')
		{
			lineNumber++;
			i++;
		}
		else if (programCodeBuffer[i] == '#')
		{
			while (programCodeBuffer[i] != '\0' && programCodeBuffer[i] != '\n')
				i++;
		}
		else if (::isspace(programCodeBuffer[i]))
		{
			while (::isspace(programCodeBuffer[i]) && programCodeBuffer[i] != '\0' && programCodeBuffer[i] != '\n')
				i++;
		}
		else if (::isalpha(programCodeBuffer[i]))
		{
			token.type = Token::IDENTIFIER;
			while ((::isalpha(programCodeBuffer[i]) || ::isdigit(programCodeBuffer[i]) || programCodeBuffer[i] == '_') && programCodeBuffer[i] != '\0' && programCodeBuffer[i] != '\n')
				token.text += programCodeBuffer[i++];
		}
		else if (::isdigit(programCodeBuffer[i]))
		{
			token.type = Token::NUMBER;
			while ((::isdigit(programCodeBuffer[i]) || programCodeBuffer[i] == '.') && programCodeBuffer[i] != '\0' && programCodeBuffer[i] != '\n')
				token.text += programCodeBuffer[i++];
		}
		else if (this->IsAnyChar(programCodeBuffer[i], "=~!%*/-+<>"))
		{
			token.type = Token::OPERATOR;
			token.text = programCodeBuffer[i++];
			if (this->IsAnyChar(token.text.c_str()[0], "%*/-+<>") && programCodeBuffer[i] == '=')
				token.text += programCodeBuffer[i++];
		}
		else if (this->IsAnyChar(programCodeBuffer[i], ",;"))
		{
			token.type = Token::DELIMETER;
			token.text = programCodeBuffer[i++];
		}
		else if (this->IsAnyChar(programCodeBuffer[i], "{(["))
		{
			token.type = Token::OPENER;
			token.text = programCodeBuffer[i++];
		}
		else if (this->IsAnyChar(programCodeBuffer[i], "})]"))
		{
			token.type = Token::CLOSER;
			token.text = programCodeBuffer[i++];
		}
		else if (programCodeBuffer[i] == '"')
		{
			token.type = Token::STRING;
			i++;
			while (programCodeBuffer[i] != '"')
			{
				if (programCodeBuffer[i] == '\0' || programCodeBuffer[i] == '\n')
					throw new CompileTimeException("Encountered run-away string.", lineNumber);
				token.text += programCodeBuffer[i++];
			}
		}
		else
		{
			throw new CompileTimeException("Unrecognized token \"%s\".", lineNumber);
		}

		programCodeBufferLocation = i;
		return token;
	}

	bool Tokenizer::IsAnyChar(char givenChar, const char* charSet)
	{
		for (int i = 0; charSet[i] != '\0'; i++)
			if (givenChar == charSet[i])
				return true;

		return false;
	}
}