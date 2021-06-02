#include "Parser.h"
#include "rapidjson/cursorstreamwrapper.h"
#include "rapidjson/error/en.h"
#include "Exceptions.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Powder
{
	Parser::Parser()
	{
		this->grammarDoc = nullptr;
	}

	/*virtual*/ Parser::~Parser()
	{
		delete this->grammarDoc;
	}

	Parser::SyntaxNode* Parser::Parse(const TokenList& tokenList)
	{
		// TODO: Find a better way to locate this file.
		std::string grammarFilePath = R"(E:\git_repos\Powder\Source\Compiler\Grammar.json)";
		std::fstream fileStream;
		fileStream.open(grammarFilePath, std::fstream::in);
		if (!fileStream.is_open())
		{
			std::cerr << "Failed to open file: " + grammarFilePath << std::endl;
			throw new CompileTimeException("Could not locate grammar file: %s");
		}

		std::stringstream stringStream;
		stringStream << fileStream.rdbuf();
		std::string grammarFileJsonText = stringStream.str();
		fileStream.close();

		this->grammarDoc = new rapidjson::Document();
		rapidjson::StringStream stream(grammarFileJsonText.c_str());
		rapidjson::CursorStreamWrapper<rapidjson::StringStream> streamWrapper(stream);

		if (this->grammarDoc->ParseStream(streamWrapper).HasParseError())
		{
			rapidjson::ParseErrorCode errorCode = this->grammarDoc->GetParseError();
			const char* jsonParseError = rapidjson::GetParseError_En(errorCode);
			size_t line = streamWrapper.GetLine();
			size_t column = streamWrapper.GetColumn();

			throw new CompileTimeException("Grammar file has JSON parser error on line %d, column %d: %s");
		}

		Range range = { tokenList.GetHead(), tokenList.GetTail(), tokenList.GetCount() };
		SyntaxNode* rootNode = this->TryGrammarRule("statement-list", range);
		if(rootNode)
			rootNode->FlattenWherePossible();

		while (rootNode->PerformReductions())
		{
		}

		return rootNode;
	}

	Parser::SyntaxNode* Parser::TryGrammarRule(const char* nonTerminal, const Range& range)
	{
		if (this->IsNonTerminal(nonTerminal))
		{
			const rapidjson::Value& expansionsListValue = (*this->grammarDoc)[nonTerminal];
			if (!expansionsListValue.IsArray())
				throw new CompileTimeException("Expected array in grammar JSON file for non-terminal production rules.");
			
			for (int i = 0; i < (signed)expansionsListValue.Size(); i++)
			{
				SyntaxNode* syntaxNode = this->TryExpansionRule(nonTerminal, expansionsListValue[i], range);
				if (syntaxNode)
					return syntaxNode;
			}
		}

		return nullptr;
	}

	Parser::SyntaxNode* Parser::TryExpansionRule(const char* nonTerminal, const rapidjson::Value& matchListValue, const Range& range)
	{
		if(range.size == 1)
		{
			int b = 0;
		}

		if (!matchListValue.IsArray())
			throw new CompileTimeException("Expected expansion rule for non-terminal %s to be an array of strings in the JSON grammar file.");

		if (matchListValue.Size() == 0)
			throw new CompileTimeException("Expected expansion rule for non-terminal %s to be an array of non-zero size in the JSON grammar file.");

		struct Subsequence
		{
			std::string name;
			Range range;
		};

		int contiguousNonTerminalCount = 0;
		std::vector<Subsequence> subsequenceArray;
		for (int i = 0; i < (signed)matchListValue.Size(); i++)
		{
			const rapidjson::Value& matchValue = matchListValue[i];
			if (!matchValue.IsString())
				throw new CompileTimeException("Encountered non-string in expansion rule for non-terminal %s of JSON grammar file.");

			Subsequence subsequence;
			subsequence.name = matchValue.GetString();
			subsequence.range.firstNode = nullptr;
			subsequence.range.lastNode = nullptr;
			subsequence.range.size = 0;

			if (this->IsNonTerminal(subsequence.name.c_str()))
			{
				if (++contiguousNonTerminalCount == 2)
				{
					// Throw an exception here.  I don't allow non-terminals to be adjacent to one another in an expansion rule.
					// I don't know how to overcome this limitation, because it produces an ambiguity in my mind that I do not know how to resolve in all cases.
					throw new CompileTimeException("Encountered adjacent non-terminals in grammar file.  Look for \"%s\".");
				}
			}
			else
			{
				contiguousNonTerminalCount = 0;

				// If we don't find the terminal, then this expansion rule does not apply.
				subsequence.range.firstNode = this->FindTerminal(range, subsequence.name);
				if (!subsequence.range.firstNode)
					return nullptr;

				subsequence.range.lastNode = subsequence.range.firstNode;
				subsequence.range.size = 1;
			}

			subsequenceArray.push_back(subsequence);
		}

		// The non-terminals now simply fill the space between the terminals.
		for (int i = 0; i < (signed)subsequenceArray.size(); i++)
		{
			Subsequence& subsequence = subsequenceArray[i];
			if (this->IsNonTerminal(subsequence.name.c_str()))
			{
				// An expansion rule also doesn't apply if a non-terminal wasn't able to fill any space.
				if (i > 0 && i < (signed)subsequenceArray.size() - 1)
					if (subsequenceArray[i - 1].range.lastNode->GetNext() == subsequenceArray[i + 1].range.firstNode)
						return nullptr;

				if (i == 0)
					subsequence.range.firstNode = range.firstNode;
				else
					subsequence.range.firstNode = subsequenceArray[i - 1].range.lastNode->GetNext();

				if (i == subsequenceArray.size() - 1)
					subsequence.range.lastNode = range.lastNode;
				else
					subsequence.range.lastNode = subsequenceArray[i + 1].range.firstNode->GetPrev();

				// Again, an expansion rule also doesn't apply if a non-terminal wasn't able to fill any space.
				if (subsequence.range.firstNode == nullptr || subsequence.range.lastNode == nullptr)
					return nullptr;

				const TokenList::Node* node = subsequence.range.firstNode;
				while (true)
				{
					subsequence.range.size++;
					if (node == subsequence.range.lastNode)
						break;
					node = node->GetNext();
				}
			}
		}

		// The last check for expansion rule applicability is to verify that we have completely covered the given range.
		uint32_t totalSize = 0;
		for (int i = 0; i < (signed)subsequenceArray.size(); i++)
			totalSize += subsequenceArray[i].range.size;
		if (totalSize != range.size)
			return nullptr;

		// Lastly, recursively descend on all the non-terminals.
		SyntaxNode* parentNode = new SyntaxNode(nonTerminal);
		for (int i = 0; i < (signed)subsequenceArray.size(); i++)
		{
			Subsequence& subsequence = subsequenceArray[i];
			if (this->IsNonTerminal(subsequence.name.c_str()))
			{
				SyntaxNode* childNode = this->TryGrammarRule(subsequence.name.c_str(), subsequence.range);
				if (childNode)
					parentNode->childList.AddTail(childNode);
				else
				{
					delete childNode;
					delete parentNode;
					parentNode = nullptr;
					break;
				}
			}
			else
			{
				SyntaxNode* childNode = new SyntaxNode(subsequence.name.c_str());
				parentNode->childList.AddTail(childNode);

				// Terminals should always take up just one node in the token list.
				if (subsequence.name == "identifier" || subsequence.name == "string-literal" || subsequence.name == "number-literal")
					childNode->childList.AddTail(new SyntaxNode(subsequence.range.firstNode->value.text.c_str()));
			}
		}

		return parentNode;
	}

	bool Parser::IsNonTerminal(const char* name)
	{
		return this->grammarDoc->HasMember(name);
	}

	bool Parser::IsTerminal(const char* name)
	{
		return !this->IsNonTerminal(name);
	}

	const TokenList::Node* Parser::FindTerminal(const Range& range, std::string& terminal)
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
			node = range.firstNode;
		else if (searchDirection == SearchDirection::RIGHT_TO_LEFT)
			node = range.lastNode;

		bool found = false;
		int level = 0;
		while (true)
		{
			if ((searchDirection == SearchDirection::LEFT_TO_RIGHT && node->value.type == Token::CLOSER) || (searchDirection == SearchDirection::RIGHT_TO_LEFT && node->value.type == Token::OPENER))
				level--;

			if (level == 0)
			{
				if ((matchMode == MatchMode::MATCH_TEXT && node->value.text == terminal) || (matchMode == MatchMode::MATCH_TYPE && node->value.type == tokenType))
				{
					found = true;
					break;
				}
			}

			if ((searchDirection == SearchDirection::LEFT_TO_RIGHT && node->value.type == Token::OPENER) || (searchDirection == SearchDirection::RIGHT_TO_LEFT && node->value.type == Token::CLOSER))
				level++;

			if (searchDirection == SearchDirection::LEFT_TO_RIGHT)
			{
				if (node == range.lastNode)
					break;
				node = node->GetNext();
			}
			else if (searchDirection == SearchDirection::RIGHT_TO_LEFT)
			{
				if (node == range.firstNode)
					break;
				node = node->GetPrev();
			}
		}

		return found ? node : nullptr;
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

	void Parser::SyntaxNode::Print(std::ostream& stream, uint32_t indentLevel /*= 0*/) const
	{
		for (uint32_t i = 0; i < indentLevel; i++)
			stream << "  ";
		stream << *this->name << std::endl;

		for (const LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
			node->value->Print(stream, indentLevel + 1);
	}

	void Parser::SyntaxNode::FlattenWherePossible(void)
	{
		if (*this->name == "statement-list" || *this->name == "argument-list" || *this->name == "identifier-list")
		{
			LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead();
			while(node)
			{
				LinkedList<SyntaxNode*>::Node* nextNode = node->GetNext();

				if (*node->value->name == *this->name)
				{
					while (node->value->childList.GetCount() > 0)
					{
						this->childList.InsertAfter(node, node->value->childList.GetTail()->value);
						node->value->childList.Remove(node->value->childList.GetTail());
					}

					nextNode = node->GetNext();
					this->childList.Remove(node);
				}
				
				node = nextNode;
			}
		}

		for (LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
			node->value->FlattenWherePossible();
	}

	bool Parser::SyntaxNode::PerformReductions()
	{
		bool performedReduction = false;

		for (LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
		{
			SyntaxNode* childNode = node->value;
			if (childNode->childList.GetCount() == 1 && (*childNode->childList.GetHead()->value->name == *this->name || *childNode->name == "operand"))
			{
				SyntaxNode* newChildNode = childNode->childList.GetHead()->value;
				childNode->childList.RemoveAll();
				node->value = newChildNode;
				performedReduction = true;
			}
		}

		for (LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
		{
			SyntaxNode* childNode = node->value;
			if (*childNode->name == "wrapped-expression" && childNode->childList.GetCount() == 3)
			{
				SyntaxNode* newChildNode = childNode->childList.GetHead()->GetNext()->value;
				delete childNode->childList.GetHead()->value;
				delete childNode->childList.GetTail()->value;
				childNode->childList.RemoveAll();
				node->value = newChildNode;
				performedReduction = true;
			}
		}

		{
			LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead();
			while (node)
			{
				LinkedList<SyntaxNode*>::Node* nextNode = node->GetNext();
				if (*node->value->name == "(" || *node->value->name == ")" || *node->value->name == ";")
				{
					delete node->value;
					this->childList.Remove(node);
					performedReduction = true;
				}

				node = nextNode;
			}
		}

		for (LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
			if (node->value->PerformReductions())
				performedReduction = true;

		return performedReduction;
	}
}