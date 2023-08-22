#include "Compiler.h"
#include "Value.h"
#include "Scope.h"
#include "InstructionGenerator.h"
#include "Assembler.h"
#include "PathResolver.h"
#include "Parser.h"
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
		std::string grammarFilePath = pathResolver.ResolvePath("Compiler\\Grammar.json", PathResolver::SEARCH_BASE);
		ParseParty::Grammar grammar;
		if (!grammar.ReadFile(grammarFilePath))
			throw new CompileTimeException(FormatString("Could not open grammar file: %s", grammarFilePath.c_str()));

		// TODO: Parser needs to recognize additional operators "-->", "<--", ">--" and "--<".
		ParseParty::Parser parser;
		std::string error;
		ParseParty::Parser::SyntaxNode* rootSyntaxNode = parser.Parse(std::string(programSourceCode), grammar, &error);
		if (!rootSyntaxNode)
			throw new CompileTimeException(error);
		
#if defined POWDER_DEBUG
		/*
		rootSyntaxNode->Print(std::cout);
		*/
#endif

		//while (rootNode->PerformReductions()) {}
		//while (rootNode->PerformSugarExpansions()) {}

		LinkedList<Instruction*> instructionList;
		InstructionGenerator instructionGenerator;
		instructionGenerator.GenerateInstructionList(instructionList, rootSyntaxNode);
		delete rootSyntaxNode;

		Assembler assembler;
		return assembler.AssembleExecutable(instructionList, this->generateDebugInfo);
	}

#if 0
	bool ParseParty::Parser::SyntaxNode::PerformSugarExpansions()
	{
		bool performedExpansion = false;

		std::vector<std::string> assignmentModifyArray;
		assignmentModifyArray.push_back("+=");
		assignmentModifyArray.push_back("-=");
		assignmentModifyArray.push_back("*=");
		assignmentModifyArray.push_back("/=");
		assignmentModifyArray.push_back("%=");

		if (*this->name == "binary-expression" && this->childList.GetCount() == 3)
		{
			for (int i = 0; i < (signed)assignmentModifyArray.size(); i++)
			{
				const std::string& assignmentModifier = assignmentModifyArray[i];
				SyntaxNode* assignmentModifierNode = this->childList.GetHead()->GetNext()->value;
				if (*assignmentModifierNode->text == assignmentModifier)
				{
					*this->name = "assignment-expression";
					*assignmentModifierNode->text = "=";
					std::string assignmentStr = assignmentModifier.substr(0, 1);
					SyntaxNode* syntaxNode = new SyntaxNode("binary-expression", assignmentModifierNode->fileLocation);
					syntaxNode->childList.AddTail(this->childList.GetHead()->value->Copy());
					syntaxNode->childList.AddTail(new SyntaxNode(assignmentStr.c_str(), assignmentModifierNode->fileLocation));
					syntaxNode->childList.AddTail(this->childList.GetHead()->GetNext()->GetNext()->value);
					this->childList.GetHead()->GetNext()->GetNext()->value = syntaxNode;
					performedExpansion = true;
					break;
				}
			}
		}

		if (*this->name == "member-access-expression" && this->childList.GetCount() == 3)
		{
			SyntaxNode* identifierNode = this->childList.GetHead()->GetNext()->GetNext()->value;
			if (*identifierNode->text == "identifier")
			{
				*this->name = "container-field-expression";
				delete this->childList.GetHead()->GetNext()->value;
				this->childList.Remove(this->childList.GetHead()->GetNext());
				SyntaxNode* literalNode = new SyntaxNode("literal", identifierNode->fileLocation);
				literalNode->childList.AddTail(new SyntaxNode("string-literal", identifierNode->fileLocation));
				literalNode->GetChild(0)->childList.AddTail(new SyntaxNode(identifierNode->GetChild(0)->name->c_str(), identifierNode->fileLocation));
				delete identifierNode;
				this->childList.GetHead()->GetNext()->value = literalNode;
				performedExpansion = true;
			}
		}

		for (LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
			if (node->value->PerformSugarExpansions())
				performedExpansion = true;

		return performedExpansion;
	}

	bool ParseParty::Parser::SyntaxNode::PerformReductions()
	{
		bool performedReduction = false;

		if (*this->name == "statement-list" ||
			*this->name == "argument-list" ||
			*this->name == "identifier-list" ||
			*this->name == "list-element-list" ||
			*this->name == "map-pair-list" ||
			*this->name == "capture-list")
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
			if (*childNode->text != "function-call")
			{
				if (childNode->childList.GetCount() == 1 &&
					(*childNode->text == *this->name ||
						*childNode->text == "expression" ||
						*childNode->text == "statement" ||
						*childNode->text == "embedded-statement" ||
						*childNode->text == "block" ||
						*childNode->text == "unary-expression" ||
						*childNode->text == "argument"))
				{
					SyntaxNode* newChildNode = childNode->GetChild(0);
					childNode->childList.RemoveAll();
					delete childNode;
					node->value = newChildNode;
					performedReduction = true;
				}
			}
		}

		for (LinkedList<SyntaxNode*>::Node* node = this->childList.GetHead(); node; node = node->GetNext())
		{
			SyntaxNode* childNode = node->value;
			if (*childNode->text == "wrapped-expression" && childNode->childList.GetCount() == 3)
			{
				SyntaxNode* newChildNode = childNode->GetChild(1);
				delete childNode->GetChild(0);
				delete childNode->childList.GetTail()->value;
				childNode->childList.RemoveAll();
				node->value = newChildNode;
				performedReduction = true;
			}
		}

		if (*this->name != "string-literal")
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
#endif
}