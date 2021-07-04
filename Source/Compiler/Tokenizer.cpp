#include "Tokenizer.h"
#include <ctype.h>
#include <cstdint>
#include "Exceptions.hpp"

namespace Powder
{
	Tokenizer::Tokenizer()
	{
		this->operatorArray = new std::vector<std::string>();
		this->operatorArray->push_back(".");
		this->operatorArray->push_back("=");
		this->operatorArray->push_back("+=");
		this->operatorArray->push_back("-=");
		this->operatorArray->push_back("*=");
		this->operatorArray->push_back("/=");
		this->operatorArray->push_back("%=");
		this->operatorArray->push_back("==");
		this->operatorArray->push_back("+");
		this->operatorArray->push_back("-");
		this->operatorArray->push_back("*");
		this->operatorArray->push_back("/");
		this->operatorArray->push_back("%");
		this->operatorArray->push_back(":");
		this->operatorArray->push_back("<");
		this->operatorArray->push_back("<=");
		this->operatorArray->push_back(">");
		this->operatorArray->push_back(">=");
		this->operatorArray->push_back("-->");
		this->operatorArray->push_back("--<");
		this->operatorArray->push_back("<--");
		this->operatorArray->push_back(">--");
		this->operatorArray->push_back("&&");
		this->operatorArray->push_back("||");
		this->operatorArray->push_back("!");
	}

	/*virtual*/ Tokenizer::~Tokenizer()
	{
		delete this->operatorArray;
	}

	void Tokenizer::Tokenize(const char* programCodeBuffer, TokenList& tokenList)
	{
		uint64_t programCodeBufferLocation = 0;
		FileLocation fileLocation;
		fileLocation.lineNumber = 1;
		fileLocation.columnNumber = 1;
		while (programCodeBuffer[programCodeBufferLocation] != '\0')
		{
			Token token = this->GenerateToken(programCodeBuffer, programCodeBufferLocation, fileLocation);
			if (token.type != Token::UNKNOWN)
				tokenList.AddTail(token);
		}
	}

	Token Tokenizer::GenerateToken(const char* programCodeBuffer, uint64_t& programCodeBufferLocation, FileLocation& fileLocation)
	{
		Token token;
		token.fileLocation = fileLocation;
		uint64_t i = programCodeBufferLocation;

		if (programCodeBuffer[i] == '\n')
		{
			fileLocation.lineNumber++;
			fileLocation.columnNumber = 1;
			i++;
		}
		else if (programCodeBuffer[i] == '#')
		{
			while (programCodeBuffer[i] != '\0' && programCodeBuffer[i] != '\n')
			{
				i++;
				fileLocation.columnNumber++;
			}
		}
		else if (::isspace(programCodeBuffer[i]))
		{
			while (::isspace(programCodeBuffer[i]) && programCodeBuffer[i] != '\0' && programCodeBuffer[i] != '\n')
			{
				i++;
				fileLocation.columnNumber++;
			}
		}
		else if (::isalpha(programCodeBuffer[i]))
		{
			// Note that we make no distinction here between keywords and variable names.  The instruction generator knows about the keywords and looks for them in the AST.
			token.type = Token::IDENTIFIER;
			while ((::isalpha(programCodeBuffer[i]) || ::isdigit(programCodeBuffer[i]) || programCodeBuffer[i] == '_') && programCodeBuffer[i] != '\0' && programCodeBuffer[i] != '\n')
			{
				token.text += programCodeBuffer[i++];
				fileLocation.columnNumber++;
			}
		}
		else if (::isdigit(programCodeBuffer[i]))
		{
			token.type = Token::NUMBER;
			while ((::isdigit(programCodeBuffer[i]) || programCodeBuffer[i] == '.') && programCodeBuffer[i] != '\0' && programCodeBuffer[i] != '\n')
			{
				token.text += programCodeBuffer[i++];
				fileLocation.columnNumber++;
			}
		}
		else if (this->IsAnyChar(programCodeBuffer[i], "=~!%*/-+<>&|:."))
		{
			token.type = Token::OPERATOR;
			token.text = this->SoakUpToken(programCodeBuffer, i, *this->operatorArray);
			if (token.text.length() == 0)
				throw new CompileTimeException("Failed to parse operator token.", &fileLocation);
			fileLocation.columnNumber += (uint16_t)token.text.length();
		}
		else if (this->IsAnyChar(programCodeBuffer[i], ",;"))
		{
			token.type = Token::DELIMETER;
			token.text = programCodeBuffer[i++];
			fileLocation.columnNumber++;
		}
		else if (this->IsAnyChar(programCodeBuffer[i], "{(["))
		{
			token.type = Token::OPENER;
			token.text = programCodeBuffer[i++];
			fileLocation.columnNumber++;
		}
		else if (this->IsAnyChar(programCodeBuffer[i], "})]"))
		{
			token.type = Token::CLOSER;
			token.text = programCodeBuffer[i++];
			fileLocation.columnNumber++;
		}
		else if (programCodeBuffer[i] == '"')
		{
			token.type = Token::STRING;
			i++;
			fileLocation.columnNumber++;
			while (programCodeBuffer[i] != '"')
			{
				if (programCodeBuffer[i] == '\0' || programCodeBuffer[i] == '\n')
					throw new CompileTimeException("Encountered run-away string.", &fileLocation);
				if (programCodeBuffer[i] == '\\')
				{
					if (programCodeBuffer[i + 1] == '"')
						token.text += "\"";
					else if (programCodeBuffer[i + 1] == 'n')
						token.text += "\n";
					else if (programCodeBuffer[i + 1] == 'r')
						token.text += "\r";
					else if (programCodeBuffer[i + 1] == 't')
						token.text += "\t";
					else if (programCodeBuffer[i + 1] == '\\')
						token.text += "\\";
					else
						throw new CompileTimeException(FormatString("Encountered unknown escape sequence: \\%c", programCodeBuffer[i + 1]), &fileLocation);
					i += 2;
					fileLocation.columnNumber += 2;
				}
				else
				{
					token.text += programCodeBuffer[i++];
					fileLocation.columnNumber++;
				}
			}
			i++;
			fileLocation.columnNumber++;
		}
		else
		{
			throw new CompileTimeException(FormatString("Unrecognized token \"%c\".", programCodeBuffer[i]), &fileLocation);
		}

		programCodeBufferLocation = i;
		return token;
	}

	std::string Tokenizer::SoakUpToken(const char* programCodeBuffer, uint64_t& programCodeBufferLocation, const std::vector<std::string>& optionsArray)
	{
		std::string bestTokenText;
		for (int i = 0; i < (signed)optionsArray.size(); i++)
		{
			const std::string& optionText = optionsArray[i];
			uint64_t j = programCodeBufferLocation;
			std::string tokenText = "";
			while (programCodeBuffer[j] != '\0' && programCodeBuffer[j] != '\n')
			{
				uint64_t k = j - programCodeBufferLocation;
				if (k >= optionText.length())
					break;
				if (programCodeBuffer[j] != optionText.c_str()[k])
					break;
				tokenText += programCodeBuffer[j++];
			}
			if (tokenText == optionText && tokenText.length() > bestTokenText.length())
				bestTokenText = tokenText;
		}
		programCodeBufferLocation += bestTokenText.length();
		return bestTokenText;
	}

	/*static*/ bool Tokenizer::IsAnyChar(char givenChar, const char* charSet)
	{
		for (int i = 0; charSet[i] != '\0'; i++)
			if (givenChar == charSet[i])
				return true;

		return false;
	}
}