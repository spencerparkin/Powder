#pragma once

#include "Defines.h"
#include "Tokenizer.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include <string>
#include <ostream>
#include <vector>

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
			Range();
			Range(const TokenList::Node* firstNode, const TokenList::Node* lastNode);

			uint32_t CalcSize() const;
			std::string Print() const;

			const TokenList::Node* firstNode;
			const TokenList::Node* lastNode;
		};

		SyntaxNode* TryGrammarRule(const char* nonTerminal, const Range& range);
		SyntaxNode* TryExpansionRule(const char* nonTerminal, const rapidjson::Value& matchListValue, const Range& range);

		bool IsNonTerminal(const char* name);
		bool IsTerminal(const char* name);

		enum SearchDirection
		{
			LEFT_TO_RIGHT,
			RIGHT_TO_LEFT
		};

		void FindAllRootLevelTerminals(const Range& range, std::vector<const TokenList::Node*>& terminalArray, SearchDirection searchDirection);
		const TokenList::Node* ScanTerminalsForMatch(int& i, const std::vector<const TokenList::Node*>& terminalArray, const std::string& terminal);

		rapidjson::Document* grammarDoc;
	};
}