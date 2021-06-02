#pragma once

#include "Defines.h"
#include "Tokenizer.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include <string>
#include <ostream>

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

			void Print(std::ostream& stream, uint32_t indentLevel = 0) const;
			void FlattenWherePossible(void);
			bool PerformReductions();

			LinkedList<SyntaxNode*> childList;
			std::string* name;
		};

		SyntaxNode* Parse(const TokenList& tokenList);

	private:

		struct Range
		{
			const TokenList::Node* firstNode;
			const TokenList::Node* lastNode;
			uint32_t size;
		};

		SyntaxNode* TryGrammarRule(const char* nonTerminal, const Range& range);
		SyntaxNode* TryExpansionRule(const char* nonTerminal, const rapidjson::Value& matchListValue, const Range& range);

		bool IsNonTerminal(const char* name);
		bool IsTerminal(const char* name);

		const TokenList::Node* FindTerminal(const Range& range, std::string& terminal);

		rapidjson::Document* grammarDoc;
	};
}