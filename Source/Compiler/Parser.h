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
			SyntaxNode(const char* name, const FileLocation& fileLocation);
			virtual ~SyntaxNode();

			void Print(std::ostream& stream, uint32_t indentLevel = 0) const;
			bool PerformReductions();
			void PatchParentPointers();

			const SyntaxNode* FindChild(const std::string& name, uint32_t maxRecurseDepth, uint32_t depth = 1) const;
			const SyntaxNode* FindParent(const std::string& name, uint32_t maxRecurseDepth, uint32_t depth = 1) const;

			SyntaxNode* parentNode;
			LinkedList<SyntaxNode*> childList;
			std::string* name;
			FileLocation fileLocation;
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

		struct ParseError
		{
			ParseError()
			{
				this->depth = 0;
				this->matchCount = 0;
			}

			Range range;
			std::string sourceCode;
			std::string grammarRule;
			std::string expansionRule;
			std::string reason;
			uint32_t matchCount;
			uint32_t depth;

			void Detail(const Range& range, const char* nonTerminal, const std::string& reason, const rapidjson::Value& matchListValue, uint32_t matchCount, uint32_t depth);
			void ThrowException();
			void Reset();
		};

		SyntaxNode* TryGrammarRule(const char* nonTerminal, const Range& range, ParseError& parseError, uint32_t depth);
		SyntaxNode* TryExpansionRule(const char* nonTerminal, const rapidjson::Value& matchListValue, const Range& range, ParseError& parseError, uint32_t depth);

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