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
#include "Error.h"
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
		this->syntaxHandlerMap.Insert("@identifier", new IdentifierExpressionHandler());
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

	// Note that the fundamental difference between an expression and a statement is
	// that the former always leaves a new value on the eval stack, while the latter
	// leaves it alone.  Both may use the stack, but the net result of a statement
	// should be the eval stack left untouched, while that of an expression is to
	// leave some new result of a computation.
	bool InstructionGenerator::GenerateInstructionList(LinkedList<Instruction*>& instructionList, const ParseParty::Parser::SyntaxNode* rootSyntaxNode, Error& error)
	{
		if (!this->GenerateInstructionListRecursively(instructionList, rootSyntaxNode, error))
		{
			error.Add("Recursive instruction generation ultimately failed.");
			return false;
		}

		// Always end the program with a halt instruction.  This is so that the last program construct has something to jump to, if needed.
		SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>(rootSyntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = SysCallInstruction::SysCall::EXIT;
		sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
		instructionList.AddTail(sysCallInstruction);

		return true;
	}

	bool InstructionGenerator::GenerateInstructionListRecursively(LinkedList<Instruction*>& instructionList, const ParseParty::Parser::SyntaxNode* syntaxNode, Error& error)
	{
		const char* name = syntaxNode->text->c_str();
		if (::strcmp(name, "empty-block") == 0)
			return true;

		SyntaxHandler* syntaxHandler = this->syntaxHandlerMap.Lookup(name);
		if (!syntaxHandler)
		{
			error.Add(std::string(syntaxNode->fileLocation) + std::format("No syntax handler found for AST node with name \"{}\".", syntaxNode->text->c_str()));
			return false;
		}

		return syntaxHandler->HandleSyntaxNode(syntaxNode, instructionList, this, error);
	}

	// In general, statements keep the eval-stack untouched, while expressions can change it.
	// There are, however, a few exceptions to this rule, and the knowledge of those exceptions
	// is embedded here, and determined by looking at the node in context.  I'm not entirely
	// happy with this mechanism, because it doesn't seem fool-proof.  Nevertheless, I keep
	// tweaking this function to compensate for bugs that I find.  If it becomes too unmanagable,
	// then I'll have to try to think of a cleaner approach to the problem.
	bool InstructionGenerator::SyntaxHandler::PopNeededForExpression(const ParseParty::Parser::SyntaxNode* syntaxNode)
	{
		if (syntaxNode->parentNode)
		{
			if (*syntaxNode->parentNode->text == "return-statement" || *syntaxNode->parentNode->text == "while-statement")
				return false;

			int i = (int)syntaxNode->parentNode->text->find("statement");
			if (i >= 0)
				return true;
		}

		return false;
	}
}