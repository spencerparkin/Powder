#include "Compiler.h"
#include "Value.h"
#include "Scope.h"
#include "InstructionGenerator.h"
#include "Assembler.h"
#include "PathResolver.h"
#include "Grammar.h"
#include "Exceptions.hpp"
#include <iostream>

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

	/*virtual*/ Executable* Compiler::CompileCode(const char* programSourceCode)
	{
		// To begin, load up our grammar file.
		std::string error;
		std::string grammarFilePath = pathResolver.ResolvePath("Compiler\\Grammar.json", PathResolver::SEARCH_BASE);
		ParseParty::Grammar grammar;
		if (!grammar.ReadFile(grammarFilePath, error))
			throw new CompileTimeException(FormatString("Could not read grammar file: %s\n\n%s", grammarFilePath.c_str(), error.c_str()));

		// Next, configure the lexer.
		ParseParty::Parser parser;
		std::string lexiconFilePath = pathResolver.ResolvePath("Compiler\\Lexicon.json", PathResolver::SEARCH_BASE);
		if (!parser.lexer.ReadFile(lexiconFilePath, error))
			throw new CompileTimeException(FormatString("Could not read lexicon file: %s\n\n%s", lexiconFilePath.c_str(), error.c_str()));

		// It's a parse-party, brah!
		ParseParty::Parser::SyntaxNode* rootSyntaxNode = parser.Parse(std::string(programSourceCode), grammar, &error);
		if (!rootSyntaxNode)
			throw new CompileTimeException(error);
		
#if defined POWDER_DEBUG
		rootSyntaxNode->Print(std::cout);
#endif

		// Perform some post-processing on the AST.
		while (this->PerformReductions(rootSyntaxNode)) {}
		while (this->PerformSugarExpansions(rootSyntaxNode)) {}

		// Organize the program as a sequence of instructions.
		LinkedList<Instruction*> instructionList;
		InstructionGenerator instructionGenerator;
		instructionGenerator.GenerateInstructionList(instructionList, rootSyntaxNode);
		delete rootSyntaxNode;

		// And finally, go assemble the program into an executable.
		Assembler assembler;
		return assembler.AssembleExecutable(instructionList, this->generateDebugInfo);
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
					performedExpansion = true;
					break;
				}
			}
		}

		if (*parentNode->text == "member-access-expression" && parentNode->GetChildCount() == 3)
		{
			ParseParty::Parser::SyntaxNode* identifierNode = parentNode->GetChild(2);
			if (*identifierNode->text == "identifier")
			{
				*parentNode->text = "container-field-expression";
				parentNode->DelChild(1);
				ParseParty::Parser::SyntaxNode* literalNode = new ParseParty::Parser::SyntaxNode("literal", identifierNode->fileLocation);
				literalNode->childList->push_back(new ParseParty::Parser::SyntaxNode("string-literal", identifierNode->fileLocation));
				literalNode->GetChild(0)->childList->push_back(new ParseParty::Parser::SyntaxNode(*identifierNode->GetChild(0)->text, identifierNode->fileLocation));
				delete identifierNode;
				parentNode->SetChild(1, literalNode);
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
					newChildNode->WipeChildren();
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
				delete childNode;
				*iter = newChildNode;
				performedReduction = true;
			}
		}

		if (*parentNode->text != "string-literal")
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