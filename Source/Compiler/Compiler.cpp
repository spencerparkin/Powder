#include "Compiler.h"
#include "Value.h"
#include "Scope.h"
#include "InstructionGenerator.h"
#include "Assembler.h"
#include "PathResolver.h"
#include "Grammar.h"
#include <iostream>
#include <format>

namespace Powder
{
	Compiler theDefaultCompiler;

	Compiler::Compiler()
	{
		this->generateDebugInfo = true;
	}

	/*virtual*/ Compiler::~Compiler()
	{
	}

	/*virtual*/ Executable* Compiler::CompileCode(const char* programSourceCode, Error& error)
	{
		// To begin, load up our grammar file.
		std::string errorStr;
		std::string grammarFilePath = pathResolver.ResolvePath("Compiler\\Grammar.json", PathResolver::SEARCH_BASE, error);
		if (grammarFilePath.size() == 0)
			return nullptr;
		ParseParty::Grammar grammar;
		if (!grammar.ReadFile(grammarFilePath, errorStr))
		{
			error.Add(std::format("Could not read grammar file {} because: {}", grammarFilePath.c_str(), errorStr.c_str()));
			return nullptr;
		}

		// Next, configure the lexer.
		ParseParty::Parser parser;
		std::string lexiconFilePath = pathResolver.ResolvePath("Compiler\\Lexicon.json", PathResolver::SEARCH_BASE, error);
		if (lexiconFilePath.size() == 0)
			return nullptr;
		if (!parser.lexer.ReadFile(lexiconFilePath, errorStr))
		{
			error.Add(std::format("Could not read lexicon file {} because: {}", lexiconFilePath.c_str(), errorStr.c_str()));
			return nullptr;
		}

		// It's a parse-party, brah!
		ParseParty::Parser::SyntaxNode* rootSyntaxNode = parser.Parse(std::string(programSourceCode), grammar, &errorStr);
		if (!rootSyntaxNode)
		{
			error.Add(errorStr);
			return nullptr;
		}
		
#if defined POWDER_DEBUG
		rootSyntaxNode->Print(std::cout);
#endif

		// Perform some post-processing on the AST.
		while (this->PerformReductions(rootSyntaxNode)) {}
		while (this->PerformSugarExpansions(rootSyntaxNode)) {}

#if defined POWDER_DEBUG
		ParseParty::Parser::SyntaxNode::WriteToFile("ast.json", rootSyntaxNode);
#endif

		// Organize the program as a sequence of instructions.
		LinkedList<Instruction*> instructionList;
		InstructionGenerator instructionGenerator;
		bool generated = instructionGenerator.GenerateInstructionList(instructionList, rootSyntaxNode, error);
		delete rootSyntaxNode;
		if (!generated)
		{
			DeleteList<Instruction*>(instructionList);
			error.Add("Failed to generate program instructions!");
			return nullptr;
		}

		// And finally, go assemble the program into an executable.
		Assembler assembler;
		Executable* executable = assembler.AssembleExecutable(instructionList, this->generateDebugInfo, error);
		DeleteList<Instruction*>(instructionList);
		return executable;
	}

	bool Compiler::PerformSugarExpansions(ParseParty::Parser::SyntaxNode* parentNode)
	{
		bool performedExpansion = false;

		std::vector<std::string> assignmentModifyArray;
		assignmentModifyArray.push_back("+=");
		assignmentModifyArray.push_back("-=");
		assignmentModifyArray.push_back("*=");
		assignmentModifyArray.push_back("/=");
		assignmentModifyArray.push_back("%=");

		if (*parentNode->text == "binary-expression" && parentNode->GetChildCount() == 3)
		{
			for (int i = 0; i < (signed)assignmentModifyArray.size(); i++)
			{
				const std::string& assignmentModifier = assignmentModifyArray[i];
				ParseParty::Parser::SyntaxNode* assignmentModifierNode = parentNode->GetChild(1);
				if (*assignmentModifierNode->text == assignmentModifier)
				{
					*parentNode->text = "assignment-expression";
					*assignmentModifierNode->text = "=";
					std::string assignmentStr = assignmentModifier.substr(0, 1);
					ParseParty::Parser::SyntaxNode* syntaxNode = new ParseParty::Parser::SyntaxNode("binary-expression", assignmentModifierNode->fileLocation);
					syntaxNode->childList->push_back(parentNode->GetChild(0)->Clone());
					syntaxNode->childList->push_back(new ParseParty::Parser::SyntaxNode(assignmentStr.c_str(), assignmentModifierNode->fileLocation));
					syntaxNode->childList->push_back(parentNode->GetChild(2));
					parentNode->SetChild(2, syntaxNode);
					syntaxNode->parentNode = parentNode;
					performedExpansion = true;
					break;
				}
			}
		}

		if (*parentNode->text == "member-access-expression" && parentNode->GetChildCount() == 3)
		{
			ParseParty::Parser::SyntaxNode* identifierNode = parentNode->GetChild(2);
			if (*identifierNode->text == "@identifier")
			{
				*parentNode->text = "container-field-expression";
				parentNode->DelChild(1);
				ParseParty::Parser::SyntaxNode* literalNode = new ParseParty::Parser::SyntaxNode("literal", identifierNode->fileLocation);
				literalNode->childList->push_back(new ParseParty::Parser::SyntaxNode("@string", identifierNode->fileLocation));
				literalNode->GetChild(0)->childList->push_back(new ParseParty::Parser::SyntaxNode(*identifierNode->GetChild(0)->text, identifierNode->fileLocation));
				delete identifierNode;
				parentNode->SetChild(1, literalNode);
				literalNode->parentNode = parentNode;
				performedExpansion = true;
			}
		}

		for (ParseParty::Parser::SyntaxNode* childNode : *parentNode->childList)
			if (this->PerformSugarExpansions(childNode))
				performedExpansion = true;

		return performedExpansion;
	}

	bool Compiler::PerformReductions(ParseParty::Parser::SyntaxNode* parentNode)
	{
		bool performedReduction = false;

		for (std::list<ParseParty::Parser::SyntaxNode*>::iterator iter = parentNode->childList->begin(); iter != parentNode->childList->end(); iter++)
		{
			ParseParty::Parser::SyntaxNode* childNode = *iter;
			if (*childNode->text != "function-call")
			{
				if (childNode->GetChildCount() == 1 &&
					(*childNode->text == *parentNode->text ||
						*childNode->text == "expression" ||
						*childNode->text == "statement" ||
						*childNode->text == "embedded-statement" ||
						*childNode->text == "block" ||
						*childNode->text == "unary-expression" ||
						*childNode->text == "argument"))
				{
					ParseParty::Parser::SyntaxNode* newChildNode = childNode->GetChild(0);
					childNode->SetChild(0, nullptr);
					newChildNode->parentNode = parentNode;
					delete childNode;
					*iter = newChildNode;
					performedReduction = true;
				}
			}
		}

		for (std::list<ParseParty::Parser::SyntaxNode*>::iterator iter = parentNode->childList->begin(); iter != parentNode->childList->end(); iter++)
		{
			ParseParty::Parser::SyntaxNode* childNode = *iter;
			if (*childNode->text == "wrapped-expression" && childNode->GetChildCount() == 3)
			{
				ParseParty::Parser::SyntaxNode* newChildNode = childNode->GetChild(1);
				childNode->SetChild(1, nullptr);
				newChildNode->parentNode = parentNode;
				delete childNode;
				*iter = newChildNode;
				performedReduction = true;
			}
		}

		if (*parentNode->text != "@string")
		{
			std::list<ParseParty::Parser::SyntaxNode*>::iterator nextIter;
			for (std::list<ParseParty::Parser::SyntaxNode*>::iterator iter = parentNode->childList->begin(); iter != parentNode->childList->end(); iter = nextIter)
			{
				nextIter = iter;
				nextIter++;

				ParseParty::Parser::SyntaxNode* childNode = *iter;
				if (*childNode->text == "(" || *childNode->text == ")" ||
					*childNode->text == ";" || *childNode->text == "," ||
					*childNode->text == "{" || *childNode->text == "}" ||
					*childNode->text == "[" || *childNode->text == "]")
				{
					delete childNode;
					parentNode->childList->erase(iter);
					performedReduction = true;
				}
			}
		}

		for (ParseParty::Parser::SyntaxNode* childNode : *parentNode->childList)
			if (this->PerformReductions(childNode))
				performedReduction = true;

		return performedReduction;
	}
}