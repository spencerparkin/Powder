#include "Parser.h"
#include "rapidjson/cursorstreamwrapper.h"
#include "rapidjson/error/en.h"
#include "Exceptions.hpp"
#include "StringFormat.h"
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
			throw new CompileTimeException(FormatString("Could not open grammar file: %s", grammarFilePath.c_str()));

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
			throw new CompileTimeException(FormatString("Grammar file has JSON parser error on line %d, column %d: %s", line, column, jsonParseError));
		}

		LinkedList<ParseError> parseErrorList;
		Range range(tokenList.GetHead(), tokenList.GetTail());
		SyntaxNode* rootNode = this->TryGrammarRule("statement-list", range, parseErrorList);
		if (!rootNode)
		{
			if (parseErrorList.GetCount() == 0)
				throw new CompileTimeException("Unknown parse error.");
			else
			{
				// This hueristic might give better results, but it is still unclear
				// as to whether this will always give a file location where the
				// real parse error exists, or whether it will bog down as the size
				// of the source code scales up.  Note that one possible improvement
				// might be to check among the first several errors: which one involved
				// a grammar rule that matched the most, but not quite.
				parseErrorList.BubbleSort();
				ParseError parseError = parseErrorList.GetHead()->value;
				throw parseError.MakeException();
			}
		}
		else
		{
			// TODO: Before running reductions on the AST, we might perform some sort of
			//       analysis to make sure that it all makes sense.  For example, we wouldn't
			//       want a statement-list to appear as the conditional part of an if-statement,
			//       or for a function-definition to appear as an argument to a function.
			//       The grammar production rules prevent such a thing from ever happening,
			//       but there might be other cases to account for.

			while (rootNode->PerformReductions())
			{
			}

			rootNode->PatchParentPointers();
		}

		return rootNode;
	}

	Parser::SyntaxNode* Parser::TryGrammarRule(const char* nonTerminal, const Range& range, LinkedList<ParseError>& parseErrorList)
	{
		LinkedList<ParseError> nestedParseErrorList;

		if (this->IsNonTerminal(nonTerminal))
		{
			const rapidjson::Value& expansionsListValue = (*this->grammarDoc)[nonTerminal];
			if (!expansionsListValue.IsArray())
				throw new CompileTimeException(FormatString("Expected expansion rule for grammar rule %s in grammar JSON file to be an array.", nonTerminal));
			
			// Note that the order in which the expansion rules are applied is
			// critical to the correct parsing of the code.
			for (int i = 0; i < (signed)expansionsListValue.Size(); i++)
			{
				SyntaxNode* syntaxNode = this->TryExpansionRule(nonTerminal, expansionsListValue[i], range, nestedParseErrorList);
				if (syntaxNode)
					return syntaxNode;
			}
		}

		parseErrorList.Append(nestedParseErrorList);
		return nullptr;
	}

	Parser::SyntaxNode* Parser::TryExpansionRule(const char* nonTerminal, const rapidjson::Value& matchListValue, const Range& range, LinkedList<ParseError>& parseErrorList)
	{
		if (!matchListValue.IsArray())
			throw new CompileTimeException(FormatString("Expected expansion rule for non-terminal %s to be an array of strings in the JSON grammar file.", nonTerminal));

		if (matchListValue.Size() == 0)
			throw new CompileTimeException(FormatString("Expected expansion rule for non-terminal %s to be an array of non-zero size in the JSON grammar file.", nonTerminal));

		std::vector<const TokenList::Node*> terminalArray;
		this->FindAllRootLevelTerminals(range, terminalArray);

		// Special case: If we're trying to parse something as an expression, early-out if any top-level terminal is a semi-colon.
		// I'm hoping this doesn't come back to bite me if I do find a legitimate reason to have semi-colons in an expression.
		// In that case, I can simply remove this.  It's not just an optimization, but it may also improve error detection accuracy and applicability.
		if (::strcmp(nonTerminal, "expression") == 0)
		{
			for (int i = 0; i < (signed)terminalArray.size(); i++)
			{
				if (terminalArray[i]->value.text == ";")
				{
					ParseError parseError(range, nonTerminal, "No expression should contain semi-colons at the top-level.  (They might appear nested, but not at the top level.)", matchListValue);
					parseErrorList.AddTail(parseError);
					return nullptr;
				}
			}
		}

		struct Subsequence
		{
			std::string name;
			Range range;
		};

		int matchListSize = (signed)matchListValue.Size();
		if (matchListSize == 0)
			throw new CompileTimeException(FormatString("Encountered match list of zero size in expansin rule for non-terminal %s of JSON grammar file.", nonTerminal));

		// Do we match the rule left to right, or right to left?
		int i = 0, j = 0;
		int delta = 1;
		if (!matchListValue[matchListSize - 1].IsString())
		{
			matchListSize--;
			i = matchListSize - 1;
			j = terminalArray.size() - 1;
			delta = -1;
		}
		
		int contiguousNonTerminalCount = 0;
		std::vector<Subsequence> subsequenceArray;
		while (0 <= i && i < matchListSize)
		{
			const rapidjson::Value& matchValue = matchListValue[i];
			if (!matchValue.IsString())
				throw new CompileTimeException(FormatString("Encountered non-string in expansion rule for non-terminal %s of JSON grammar file.", nonTerminal));

			Subsequence subsequence;
			subsequence.name = matchValue.GetString();

			if (this->IsNonTerminal(subsequence.name.c_str()))
			{
				if (++contiguousNonTerminalCount == 2)
				{
					// Throw an exception here.  I don't allow non-terminals to be adjacent to one another in an expansion rule.
					// I don't know how to overcome this limitation, because it produces an ambiguity in my mind that I do not know how to resolve in all cases.
					// One thought is to expand a grammar rule until it has no two non-terminals touching one another, but that adds extra complication, and it's
					// more likely that I just don't understand the proper algorithms for dealing with expansion rules.
					throw new CompileTimeException(FormatString("Encountered adjacent non-terminals in expansion rule for grammar rule: %s", nonTerminal));
				}
			}
			else
			{
				contiguousNonTerminalCount = 0;

				// If we don't find the terminal, then this expansion rule does not apply.
				const TokenList::Node* foundNode = this->ScanTerminalsForMatch(j, delta, terminalArray, subsequence.name);
				if (!foundNode)
				{
					std::string reason = "Failed to find terminal match \"" + subsequence.name + "\" in grammar expansion rule.";
					ParseError parseError(range, nonTerminal, reason, matchListValue);
					parseErrorList.AddTail(parseError);
					return nullptr;
				}

				subsequence.range.firstNode = subsequence.range.lastNode = foundNode;
			}

			if (delta > 0)
				subsequenceArray.push_back(subsequence);
			else
				subsequenceArray.insert(subsequenceArray.begin(), subsequence);

			i += delta;
		}

		// The non-terminals now simply fill the space between the terminals.
		for (int i = 0; i < (signed)subsequenceArray.size(); i++)
		{
			Subsequence& subsequence = subsequenceArray[i];
			if (this->IsNonTerminal(subsequence.name.c_str()))
			{
				// An expansion rule also doesn't apply if a non-terminal wasn't able to fill any space.
				if (i > 0 && i < (signed)subsequenceArray.size() - 1)
				{
					if (subsequenceArray[i - 1].range.lastNode->GetNext() == subsequenceArray[i + 1].range.firstNode)
					{
						std::string reason = "Failed to find non-terminal match \"" + subsequence.name + "\" in grammar expansion rule.";
						ParseError parseError(range, nonTerminal, reason, matchListValue);
						parseErrorList.AddTail(parseError);
						return nullptr;
					}
				}

				if (i == 0)
					subsequence.range.firstNode = range.firstNode;
				else if (subsequenceArray[i - 1].range.lastNode != range.lastNode)
					subsequence.range.firstNode = subsequenceArray[i - 1].range.lastNode->GetNext();

				if (i == subsequenceArray.size() - 1)
					subsequence.range.lastNode = range.lastNode;
				else if(subsequenceArray[i + 1].range.firstNode != range.firstNode)
					subsequence.range.lastNode = subsequenceArray[i + 1].range.firstNode->GetPrev();

				// Again, an expansion rule also doesn't apply if a non-terminal wasn't able to fill any space.
				if (subsequence.range.firstNode == nullptr || subsequence.range.lastNode == nullptr)
				{
					std::string reason = "Failed to find non-terminal match \"" + subsequence.name + "\" in grammar expansion rule.";
					ParseError parseError(range, nonTerminal, reason, matchListValue);
					parseErrorList.AddTail(parseError);
					return nullptr;
				}
			}
		}

		// The last check for expansion rule applicability is to verify that we have completely covered the given range.
		uint32_t totalSize = 0;
		for (int i = 0; i < (signed)subsequenceArray.size(); i++)
			totalSize += subsequenceArray[i].range.CalcSize();
		if (totalSize != range.CalcSize())
		{
			std::string reason = "Grammar expansion rule did not account for all tokens.";
			ParseError parseError(range, nonTerminal, reason, matchListValue);
			parseErrorList.AddTail(parseError);
			return nullptr;
		}

		// Lastly, recursively descend on all the non-terminals.
		LinkedList<ParseError> nestedParseErrorList;
		SyntaxNode* parentNode = new SyntaxNode(nonTerminal, range.firstNode->value.fileLocation);
		for (int i = 0; i < (signed)subsequenceArray.size(); i++)
		{
			Subsequence& subsequence = subsequenceArray[i];
			FileLocation fileLocation = subsequence.range.firstNode->value.fileLocation;
			if (this->IsNonTerminal(subsequence.name.c_str()))
			{
				SyntaxNode* childNode = this->TryGrammarRule(subsequence.name.c_str(), subsequence.range, nestedParseErrorList);
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
				SyntaxNode* childNode = new SyntaxNode(subsequence.name.c_str(), fileLocation);
				parentNode->childList.AddTail(childNode);

				// Terminals should always take up just one node in the token list.
				if (subsequence.name == "identifier" || subsequence.name == "string-literal" || subsequence.name == "number-literal")
				{
					SyntaxNode* grandChildNode = new SyntaxNode(subsequence.range.firstNode->value.text.c_str(), fileLocation);
					childNode->childList.AddTail(grandChildNode);
				}
			}
		}

		if (!parentNode)
			parseErrorList.Append(nestedParseErrorList);

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

	const TokenList::Node* Parser::ScanTerminalsForMatch(int& i, int i_delta, const std::vector<const TokenList::Node*>& terminalArray, const std::string& terminal)
	{
		while (0 <= i && i < (signed)terminalArray.size())
		{
			const TokenList::Node* node = terminalArray[i];
			i += i_delta;

			if (terminal == "identifier")
			{
				if (node->value.type == Token::IDENTIFIER)
					return node;
			}
			else if (terminal == "string-literal")
			{
				if (node->value.type == Token::STRING)
					return node;
			}
			else if (terminal == "number-literal")
			{
				if (node->value.type == Token::NUMBER)
					return node;
			}
			else
			{
				if (terminal == node->value.text)
					return node;
			}
		}

		return nullptr;
	}

	void Parser::FindAllRootLevelTerminals(const Range& range, std::vector<const TokenList::Node*>& terminalArray)
	{
		const TokenList::Node* node = range.firstNode;
		int level = 0;
		while (true)
		{
			if (node->value.type == Token::CLOSER)
				level--;

			if (level == 0 && this->IsTerminal(node->value.text.c_str()))
				terminalArray.push_back(node);

			if (node->value.type == Token::OPENER)
				level++;

			if (node == range.lastNode)
				break;

			node = node->GetNext();
		}
	}

	Parser::Range::Range()
	{
		this->firstNode = nullptr;
		this->lastNode = nullptr;
	}

	Parser::Range::Range(const TokenList::Node* firstNode, const TokenList::Node* lastNode)
	{
		this->firstNode = firstNode;
		this->lastNode = lastNode;
	}

	uint32_t Parser::Range::CalcSize() const
	{
		uint32_t size = 0;
		const TokenList::Node* node = this->firstNode;
		while (true)
		{
			size++;
			if (node == this->lastNode)
				break;
			node = node->GetNext();
		}
		return size;
	}

	std::string Parser::Range::Print() const
	{
		std::string rangeStr;
		const TokenList::Node* node = this->firstNode;
		while (true)
		{
			if (rangeStr.length() > 0)
				rangeStr += " ";
			rangeStr += node->value.text;
			if (node == this->lastNode)
				break;
			node = node->GetNext();
		}
		return rangeStr;
	}

	Parser::SyntaxNode::SyntaxNode(const char* name, const FileLocation& fileLocation)
	{
		this->parentNode = nullptr;
		this->name = new std::string;
		*this->name = name;
		this->fileLocation = fileLocation;
	}

	/*virtual*/ Parser::SyntaxNode::~SyntaxNode()
	{
		delete this->name;
		DeleteList<SyntaxNode*>(this->childList);
	}

#if defined POWDER_DEBUG
	void Parser::SyntaxNode::Print(std::ostream& stream, uint32_t indentLevel /*= 0*/) const
	{
		for (uint32_t i = 0; i < indentLevel; i++)
			stream << "  ";
		stream << *this->name << std::endl;
		for (const LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
			node->value->Print(stream, indentLevel + 1);
	}
#endif //POWDER_DEBUG

	bool Parser::SyntaxNode::PerformReductions()
	{
		bool performedReduction = false;

		if (*this->name == "statement-list" ||
			*this->name == "argument-list" ||
			*this->name == "identifier-list" ||
			*this->name == "list-element-list" ||
			*this->name == "map-pair-list")
		{
			LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead();
			while (node)
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
					performedReduction = true;
				}

				node = nextNode;
			}
		}

		for (LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
		{
			SyntaxNode* childNode = node->value;
			if (childNode->childList.GetCount() == 1 &&
				(*childNode->childList.GetHead()->value->name == *this->name ||
					*childNode->name == "expression" ||
					*childNode->name == "statement" ||
					*childNode->name == "embedded-statement" ||
					*childNode->name == "block" ||
					*childNode->name == "unary-expression" ||
					*childNode->name == "argument"))
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
				if (*node->value->name == "(" || *node->value->name == ")" ||
					*node->value->name == ";" || *node->value->name == "," ||
					*node->value->name == "{" || *node->value->name == "}" ||
					*node->value->name == "[" || *node->value->name == "]")
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

	void Parser::SyntaxNode::PatchParentPointers()
	{
		for (LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
		{
			SyntaxNode* childNode = node->value;
			childNode->parentNode = this;
			childNode->PatchParentPointers();
		}
	}

	const Parser::SyntaxNode* Parser::SyntaxNode::FindChild(const std::string& name, uint32_t maxRecurseDepth, uint32_t depth /*= 1*/) const
	{
		for (const LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
		{
			if (*node->value->name == name)
				return node->value;

			if (depth < maxRecurseDepth)
			{
				const SyntaxNode* foundNode = node->value->FindChild(name, maxRecurseDepth, depth + 1);
				if (foundNode)
					return foundNode;
			}
		}

		return nullptr;
	}

	const Parser::SyntaxNode* Parser::SyntaxNode::FindParent(const std::string& name, uint32_t maxRecurseDepth, uint32_t depth /*= 1*/) const
	{
		if (!this->parentNode)
			return nullptr;

		if (*this->parentNode->name == name)
			return this->parentNode;

		if (depth < maxRecurseDepth)
		{
			const SyntaxNode* foundNode = this->parentNode->FindParent(name, maxRecurseDepth, depth + 1);
			if (foundNode)
				return foundNode;
		}

		return nullptr;
	}

	Parser::ParseError::ParseError()
	{
	}

	Parser::ParseError::ParseError(const Range& range, const char* nonTerminal, const std::string& reason, const rapidjson::Value& matchListValue)
	{
		this->range = range;
		this->reason = reason;
		this->grammarRule = nonTerminal;
		this->expansionRule = "";
		this->sourceCode = range.Print();

		for (uint32_t i = 0; i < matchListValue.Size(); i++)
		{
			if (matchListValue[i].IsString())
			{
				if (this->expansionRule.length() > 0)
					this->expansionRule += " ";
				this->expansionRule += matchListValue[i].GetString();
			}
		}
	}

	CompileTimeException* Parser::ParseError::MakeException() const
	{
		std::string errorMsg;
		errorMsg += "Code: " + this->sourceCode + "\n";
		errorMsg += "Grammar Rule: " + this->grammarRule + "\n";
		errorMsg += "Expansion Rule: " + this->expansionRule + "\n";
		errorMsg += "Reason: " + this->reason + "\n";
		return new CompileTimeException(errorMsg, &this->range.firstNode->value.fileLocation);
	}

	int Parser::ParseError::SortKey() const
	{
		return this->sourceCode.length();
	}
}