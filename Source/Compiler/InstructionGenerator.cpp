#include "InstructionGenerator.h"
#include "Assembler.h"
#include "StatementListHandler.h"
#include "ContainerSizeExpressionHandler.h"
#include "ContainerFieldExpressionHandler.h"
#include "DeleteFieldExpressionHandler.h"
#include "IfStatementHandler.h"
#include "WhileStatementHandler.h"
#include "DoWhileStatementHandler.h"
#include "YieldStatementHandler.h"
#include "ForkStatementHandler.h"
#include "ForLoopStatementHandler.h"
#include "AssignmentExpressionHandler.h"
#include "BinaryExpressionHandler.h"
#include "UnaryExpressionHandler.h"
#include "LiteralExpressionHandler.h"
#include "IdentifierExpressionHandler.h"
#include "ListExpressionHandler.h"
#include "MapExpressionHandler.h"
#include "MapKeysExpressionHandler.h"
#include "ListPushPopExpressionHandler.h"
#include "FunctionCallExpressionHandler.h"
#include "FunctionDefinitionExpressionHandler.h"
#include "ReturnStatementHandler.h"
#include "MembershipExpressionHandler.h"
#include "ListInstruction.h"
#include "StoreInstruction.h"
#include "PopInstruction.h"
#include "SysCallInstruction.h"
#include "PushInstruction.h"
#include "Exceptions.hpp"
#include "HashMap.hpp"

namespace Powder
{
	InstructionGenerator::InstructionGenerator()
	{
		this->syntaxHandlerMap.Insert("statement-list", new StatementListHandler());
		this->syntaxHandlerMap.Insert("container-size-expression", new ContainerSizeExpressionHandler());
		this->syntaxHandlerMap.Insert("container-field-expression", new ContainerFieldExpressionHandler());
		this->syntaxHandlerMap.Insert("delete-field-expression", new DeleteFieldExpressionHandler());
		this->syntaxHandlerMap.Insert("if-statement", new IfStatementHandler());
		this->syntaxHandlerMap.Insert("while-statement", new WhileStatementHandler());
		this->syntaxHandlerMap.Insert("do-while-statement", new DoWhileStatementHandler());
		this->syntaxHandlerMap.Insert("yield-statement", new YieldStatementHandler());
		this->syntaxHandlerMap.Insert("fork-statement", new ForkStatementHandler());
		this->syntaxHandlerMap.Insert("for-statement", new ForLoopStatementHandler());
		this->syntaxHandlerMap.Insert("assignment-expression", new AssignmentExpressionHandler());
		this->syntaxHandlerMap.Insert("binary-expression", new BinaryExpressionHandler());
		this->syntaxHandlerMap.Insert("left-unary-expression", new UnaryExpressionHandler());
		this->syntaxHandlerMap.Insert("right-unary-expression", new UnaryExpressionHandler());
		this->syntaxHandlerMap.Insert("literal", new LiteralExpressionHandler());
		this->syntaxHandlerMap.Insert("identifier", new IdentifierExpressionHandler());
		this->syntaxHandlerMap.Insert("list-literal", new ListExpressionHandler());
		this->syntaxHandlerMap.Insert("map-literal", new MapExpressionHandler());
		this->syntaxHandlerMap.Insert("map-keys-expression", new MapKeysExpressionHandler());
		this->syntaxHandlerMap.Insert("list-push-pop-expression", new ListPushPopExpressionHandler());
		this->syntaxHandlerMap.Insert("function-call", new FunctionCallExpressionHandler());
		this->syntaxHandlerMap.Insert("function-definition", new FunctionDefinitionExpressionHandler());
		this->syntaxHandlerMap.Insert("return-statement", new ReturnStatementHandler());
		this->syntaxHandlerMap.Insert("membership-expression", new MembershipExpressionHandler());
	}

	/*virtual*/ InstructionGenerator::~InstructionGenerator()
	{
		this->syntaxHandlerMap.DeleteAndClear();
	}

	void InstructionGenerator::GenerateInstructionList(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* rootSyntaxNode)
	{
		this->GenerateInstructionListRecursively(instructionList, rootSyntaxNode);

		// Always end the program with a halt instruction.  This is so that the last program construct has something to jump to, if needed.
		SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>(rootSyntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = SysCallInstruction::SysCall::EXIT;
		sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
		instructionList.AddTail(sysCallInstruction);
	}

	void InstructionGenerator::GenerateInstructionListRecursively(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode)
	{
		SyntaxHandler* syntaxHandler = this->syntaxHandlerMap.Lookup(syntaxNode->name->c_str());
		if (!syntaxHandler)
			throw new CompileTimeException(FormatString("No syntax handler found for AST node with name \"%s\".", syntaxNode->name->c_str()), &syntaxNode->fileLocation);

		syntaxHandler->HandleSyntaxNode(syntaxNode, instructionList, this);
	}
}