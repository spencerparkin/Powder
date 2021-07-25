#include "ForLoopStatementHandler.h"
#include "FunctionCallExpressionHandler.h"
#include "PushInstruction.h"
#include "ListInstruction.h"
#include "Assembler.h"

namespace Powder
{
	ForLoopStatementHandler::ForLoopStatementHandler()
	{
	}

	/*virtual*/ ForLoopStatementHandler::~ForLoopStatementHandler()
	{
	}

	/*virtual*/ void ForLoopStatementHandler::HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 3)
			throw new CompileTimeException("Expected \"for-statement\" in AST to have exactly 3 children.", &syntaxNode->fileLocation);

		const Parser::SyntaxNode* iterationNode = syntaxNode->childList.GetHead()->GetNext()->value;
		if (*iterationNode->name != "for-iteration-expression")
			throw new CompileTimeException("Expected \"for-statement\" in AST to have \"for-iteration-expression\" child.", &syntaxNode->fileLocation);

		if (iterationNode->childList.GetCount() != 3)
			throw new CompileTimeException("Exected \"for-iteration-expression\" in AST to have exactly 3 children.", &iterationNode->fileLocation);

		const Parser::SyntaxNode* identifierNode = iterationNode->childList.GetHead()->value;
		if (*identifierNode->name != "identifier")
			throw new CompileTimeException("Expected \"identifier\" in AST as child of \"for-iteration-expression\" node.", &identifierNode->fileLocation);

		// Push an argument list onto the eval stack.
		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(iterationNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = PushInstruction::DataType::EMPTY_LIST;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		instructionList.AddTail(pushInstruction);

		// Load our argument into the argument list that will ask the iterator to reset.
		pushInstruction = Instruction::CreateForAssembly<PushInstruction>(iterationNode->fileLocation);
		entry.Reset();
		entry.code = PushInstruction::DataType::STRING;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		entry.Reset();
		entry.string = "reset";
		pushInstruction->assemblyData->configMap.Insert("data", entry);
		instructionList.AddTail(pushInstruction);
		ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>(iterationNode->fileLocation);
		entry.Reset();
		entry.code = ListInstruction::Action::PUSH_RIGHT;
		listInstruction->assemblyData->configMap.Insert("action", entry);
		instructionList.AddTail(listInstruction);

		// Push the iterator function onto the eval-stack.
		const Parser::SyntaxNode* iteratorNode = iterationNode->childList.GetHead()->GetNext()->GetNext()->value;
		instructionGenerator->GenerateInstructionListRecursively(instructionList, iteratorNode);

		// Now we're ready to make the call to the iterator to reset iteration (to set it up for the first time, really.)
		FunctionCallExpressionHandler::GenerateCallInstructions(instructionList, iterationNode->fileLocation);

		// TODO: We need to add a feature to the push instruction where it can push an existing eval stack value onto the top of the stack.
	}
}