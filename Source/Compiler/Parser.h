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
	class CompileTimeException;

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

#if defined POWDER_DEBUG
			void Print(std::ostream& stream, uint32_t indentLevel = 0) const;
#endif
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
			ParseError();
			ParseError(const Range& range, const char* nonTerminal, const std::string& reason, const rapidjson::Value& matchListValue);

			CompileTimeException* MakeException() const;
			int SortKey() const;

			Range range;
			std::string sourceCode;
			std::string grammarRule;
			std::string expansionRule;
			std::string reason;
		};

		SyntaxNode* TryGrammarRule(const char* nonTerminal, const Range& range, LinkedList<ParseError>& parseErrorList);
		SyntaxNode* TryExpansionRule(const char* nonTerminal, const rapidjson::Value& matchListValue, const Range& range, LinkedList<ParseError>& parseErrorList);

		bool IsNonTerminal(const char* name);
		bool IsTerminal(const char* name);

		void FindAllRootLevelTerminals(const Range& range, std::vector<const TokenList::Node*>& terminalArray);
		const TokenList::Node* ScanTerminalsForMatch(int& i, int i_delta, const std::vector<const TokenList::Node*>& terminalArray, const std::string& terminal);

		rapidjson::Document* grammarDoc;
	};
}