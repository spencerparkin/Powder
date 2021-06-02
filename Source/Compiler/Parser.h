#pragma once

#include "Defines.h"
#include "Tokenizer.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include <string>

namespace Powder
{
	class POWDER_API Parser
	{
	public:
		Parser();
		virtual ~Parser();

		class SyntaxNode
		{
		public:
			SyntaxNode(const char* name);
			virtual ~SyntaxNode();

			LinkedList<SyntaxNode*> childList;
			std::string* name;
		};

		SyntaxNode* Parse(const TokenList& tokenList);

	private:

		SyntaxNode* TryGrammarRule(const char* nonTerminal, const TokenList& tokenList);
		SyntaxNode* TryExpansionRule(const char* nonTerminal, const rapidjson::Value& matchListValue, const TokenList& tokenList);

		bool IsNonTerminal(const char* name);
		bool IsTerminal(const char* name);

		const TokenList::Node* FindTerminal(const TokenList& tokenList, std::string& terminal);

		rapidjson::Document* grammarDoc;
	};
}