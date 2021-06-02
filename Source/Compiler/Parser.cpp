#include "Parser.h"
#include "Grammar.h"
#include "rapidjson/cursorstreamwrapper.h"
#include "rapidjson/error/en.h"
#include "Exceptions.hpp"
#include <vector>

namespace Powder
{
	Parser::Parser()
	{
		this->grammarDoc = new rapidjson::Document();
		rapidjson::StringStream stream(grammarProductionsJson);
		rapidjson::CursorStreamWrapper<rapidjson::StringStream> streamWrapper(stream);

		if (this->grammarDoc->ParseStream(streamWrapper).HasParseError())
		{
			rapidjson::ParseErrorCode errorCode = this->grammarDoc->GetParseError();
			const char* jsonParseError = rapidjson::GetParseError_En(errorCode);
			size_t line = streamWrapper.GetLine();
			size_t column = streamWrapper.GetColumn();

			// TODO: Throw an exception here.
		}
	}

	/*virtual*/ Parser::~Parser()
	{
		delete this->grammarDoc;
	}

	Parser::SyntaxNode* Parser::Parse(const TokenList& tokenList)
	{
		return this->TryGrammarRule("statement-list", tokenList);
	}

	Parser::SyntaxNode* Parser::TryGrammarRule(const char* nonTerminal, const TokenList& tokenList)
	{
		if (this->IsNonTerminal(nonTerminal))
		{
			const rapidjson::Value& expansionsListValue = (*this->grammarDoc)[nonTerminal];
			for (int i = 0; i < (signed)expansionsListValue.MemberCount(); i++)
			{
				SyntaxNode* syntaxNode = this->TryExpansionRule(nonTerminal, expansionsListValue[i], tokenList);
				if (syntaxNode)
					return syntaxNode;
			}
		}

		return nullptr;
	}

	Parser::SyntaxNode* Parser::TryExpansionRule(const char* nonTerminal, const rapidjson::Value& matchListValue, const TokenList& tokenList)
	{
		struct Subsequence
		{
			std::string name;
			const TokenList::Node* firstNode;
			const TokenList::Node* lastNode;
		};

		int contiguousNonTerminalCount = 0;
		std::vector<Subsequence> subsequenceArray;
		for (int i = 0; i < (signed)matchListValue.MemberCount(); i++)
		{
			const rapidjson::Value& matchValue = matchListValue[i];
			Subsequence subsequence;
			subsequence.name = matchValue.GetString();
			subsequence.firstNode = nullptr;
			subsequence.lastNode = nullptr;
			subsequenceArray.push_back(subsequence);

			if (this->IsNonTerminal(subsequence.name.c_str()))
			{
				if (++contiguousNonTerminalCount == 2)
				{
					// TODO: Throw an exception here.  I don't allow non-terminals to be adjacent to one another in an expansion rule.
					//       I don't know how to overcome this limitation, because it produces an ambiguity in my mind that I do not know how to resolve.
				}
			}
			else
			{
				contiguousNonTerminalCount = 0;

				// If we don't find the terminal, then this expansion rule does not apply.
				subsequence.firstNode = this->FindTerminal(tokenList, subsequence.name);
				if (!subsequence.firstNode)
					return nullptr;

				subsequence.lastNode = subsequence.firstNode;
			}
		}

		// The non-terminals now simply fill the empty space between the terminals.
		for (int i = 0; i < (signed)subsequenceArray.size(); i++)
		{
			Subsequence& subsequence = subsequenceArray[i];

			if (this->IsNonTerminal(subsequence.name.c_str()))
			{
				if (i == 0)
					subsequence.firstNode = tokenList.GetHead();
				else
					subsequence.firstNode = subsequenceArray[i - 1].lastNode->GetNext();

				if (i == subsequenceArray.size() - 1)
					subsequence.lastNode = tokenList.GetTail();
				else
					subsequence.lastNode = subsequenceArray[i + 1].firstNode->GetPrev();
			}
		}

		// Lastly, recursively descend on all the non-terminals.
		SyntaxNode* syntaxNode = new SyntaxNode(nonTerminal);
		for (int i = 0; i < (signed)subsequenceArray.size(); i++)
		{
			Subsequence& subsequence = subsequenceArray[i];

			if (this->IsNonTerminal(subsequence.name.c_str()))
			{
				TokenList subTokenList;
				const TokenList::Node* node = subsequence.firstNode;
				while (true)
				{
					subTokenList.AddTail(node->value);
					if (node == subsequence.lastNode)
						break;
					node = node->GetNext();
				}

				SyntaxNode* childNode = this->TryGrammarRule(subsequence.name.c_str(), subTokenList);
				if (childNode)
					syntaxNode->childList.AddTail(childNode);
				else
				{
					delete childNode;
					delete syntaxNode;
					syntaxNode = nullptr;
					break;
				}
			}
		}

		return syntaxNode;
	}

	bool Parser::IsNonTerminal(const char* name)
	{
		return this->grammarDoc->HasMember(name);
	}

	bool Parser::IsTerminal(const char* name)
	{
		return !this->IsNonTerminal(name);
	}

	const TokenList::Node* Parser::FindTerminal(const TokenList& tokenList, std::string& terminal)
	{
		const TokenList::Node* node = nullptr;

		enum MatchMode
		{
			MATCH_TEXT,
			MATCH_TYPE
		};

		MatchMode matchMode = MatchMode::MATCH_TEXT;
		Token::Type tokenType = Token::UNKNOWN;

		if (terminal == "identifier")
		{
			matchMode = MatchMode::MATCH_TYPE;
			tokenType = Token::IDENTIFIER;
		}
		else if (terminal == "string-literal")
		{
			matchMode = MatchMode::MATCH_TYPE;
			tokenType = Token::STRING;
		}
		else if (terminal == "number-literal")
		{
			matchMode = MatchMode::MATCH_TYPE;
			tokenType = Token::NUMBER;
		}

		enum SearchDirection
		{
			LEFT_TO_RIGHT,
			RIGHT_TO_LEFT
		};

		SearchDirection searchDirection = SearchDirection::LEFT_TO_RIGHT;

		// In some special cases, we need to actually search right to left in order to obey operator associativity.
		if (terminal == "=" || terminal == "^")
			searchDirection = SearchDirection::RIGHT_TO_LEFT;

		if (searchDirection == SearchDirection::LEFT_TO_RIGHT)
			node = tokenList.GetHead();
		else if (searchDirection == SearchDirection::RIGHT_TO_LEFT)
			node = tokenList.GetTail();

		int level = 0;
		while (node)
		{
			node->value.AdjustLevel(level);

			if (level == 0)
			{
				if (matchMode == MatchMode::MATCH_TEXT && node->value.text == terminal)
					break;
				else if (matchMode == MatchMode::MATCH_TYPE && node->value.type == tokenType)
					break;
			}

			if (searchDirection == SearchDirection::LEFT_TO_RIGHT)
				node = node->GetNext();
			else if (searchDirection == SearchDirection::RIGHT_TO_LEFT)
				node = node->GetPrev();
		}

		return node;
	}

	Parser::SyntaxNode::SyntaxNode(const char* name)
	{
		this->name = new std::string;
		*this->name = name;
	}

	/*virtual*/ Parser::SyntaxNode::~SyntaxNode()
	{
		delete this->name;
		DeleteList<SyntaxNode*>(this->childList);
	}
}