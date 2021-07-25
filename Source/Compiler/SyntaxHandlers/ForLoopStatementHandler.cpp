#include "ForLoopStatementHandler.h"
#include "FunctionCallExpressionHandler.h"
#include "PushInstruction.h"
#include "PopInstruction.h"
#include "ListInstruction.h"
#include "StoreInstruction.h"
#include "LoadInstruction.h"
#include "BranchInstruction.h"
#include "MathInstruction.h"
#include "JumpInstruction.h"
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

		// Push the iterator function onto the eval-stack.
		const Parser::SyntaxNode* iteratorNode = iterationNode->childList.GetHead()->GetNext()->GetNext()->value;
		instructionGenerator->GenerateInstructionListRecursively(instructionList, iteratorNode);

		// Push an argument list onto the eval stack, then push the iterator function address on top of that.
		this->GenerateInstructionForIteratorCallSetup(instructionList, "reset", iterationNode->fileLocation);

		// Now we're ready to make the call to the iterator to reset iteration (to set it up for the first time, really.)
		FunctionCallExpressionHandler::GenerateCallInstructions(instructionList, iterationNode->fileLocation);

		// The result of resetting iteration is always a value we throw away, so do that now.
		PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(iterationNode->fileLocation);
		instructionList.AddTail(popInstruction);

		// We're now ready to begin the loop instructions.  Call the iterator to generate the first/next value.
		LinkedList<Instruction*> forLoopHeadInstructionList;
		this->GenerateInstructionForIteratorCallSetup(forLoopHeadInstructionList, "next", iterationNode->fileLocation);
		FunctionCallExpressionHandler::GenerateCallInstructions(instructionList, iterationNode->fileLocation);

		// Store the value in the loop iteration variable.
		StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(iterationNode->fileLocation);
		AssemblyData::Entry entry;
		entry.string = *identifierNode->childList.GetHead()->value->name;
		storeInstruction->assemblyData->configMap.Insert("name", entry);
		forLoopHeadInstructionList.AddTail(storeInstruction);

		// Iteration ends when the iteration variable becomes undefined, so test that now.
		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(iterationNode->fileLocation);
		entry.Reset();
		entry.code = PushInstruction::UNDEFINED;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>(iterationNode->fileLocation);
		forLoopHeadInstructionList.AddTail(loadInstruction);
		entry.Reset();
		entry.string = *identifierNode->childList.GetHead()->value->name;
		loadInstruction->assemblyData->configMap.Insert("name", entry);
		forLoopHeadInstructionList.AddTail(loadInstruction);
		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(iterationNode->fileLocation);
		entry.Reset();
		entry.code = MathInstruction::NOT_EQUAL;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		forLoopHeadInstructionList.AddTail(mathInstruction);
		BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>(iterationNode->fileLocation);
		forLoopHeadInstructionList.AddTail(branchInstruction);

		// Finally, we can now execute the body of the for-loop.
		LinkedList<Instruction*> forLoopBodyInstructionList;
		instructionGenerator->GenerateInstructionListRecursively(forLoopBodyInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

		// Lastly, unconditionally jump back up to the top of the for-loop and do it all over again.
		JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(iterationNode->fileLocation);
		entry.Reset();
		entry.instruction = forLoopHeadInstructionList.GetHead()->value;
		jumpInstruction->assemblyData->configMap.Insert("jump", entry);
		entry.Reset();
		entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
		jumpInstruction->assemblyData->configMap.Insert("type", entry);
		forLoopBodyInstructionList.AddTail(jumpInstruction);

		// We can now patch our branch that takes us out of the for-loop.
		entry.Reset();
		entry.jumpDelta = forLoopBodyInstructionList.GetCount() + 1;
		branchInstruction->assemblyData->configMap.Insert("branch", entry);

		// Okay, now lay down all the loop instructions, head and body.
		instructionList.Append(forLoopHeadInstructionList);
		instructionList.Append(forLoopBodyInstructionList);

		// Lastly, pop the iterator function off the eval-stack.
		popInstruction = Instruction::CreateForAssembly<PopInstruction>(iterationNode->fileLocation);
		instructionList.AddTail(popInstruction);
	}

	void ForLoopStatementHandler::GenerateInstructionForIteratorCallSetup(LinkedList<Instruction*>& instructionList, const char* action, const FileLocation& fileLocation)
	{
		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(fileLocation);
		AssemblyData::Entry entry;
		entry.code = PushInstruction::DataType::EMPTY_LIST;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		instructionList.AddTail(pushInstruction);

		pushInstruction = Instruction::CreateForAssembly<PushInstruction>(fileLocation);
		entry.Reset();
		entry.code = PushInstruction::DataType::STRING;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		entry.Reset();
		entry.string = action;
		pushInstruction->assemblyData->configMap.Insert("data", entry);
		instructionList.AddTail(pushInstruction);

		ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>(fileLocation);
		entry.Reset();
		entry.code = ListInstruction::Action::PUSH_RIGHT;
		listInstruction->assemblyData->configMap.Insert("action", entry);
		instructionList.AddTail(listInstruction);

		pushInstruction = Instruction::CreateForAssembly<PushInstruction>(fileLocation);
		entry.Reset();
		entry.code = PushInstruction::DataType::EXISTING_VALUE;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		entry.Reset();
		entry.offset = -1;
		pushInstruction->assemblyData->configMap.Insert("data", entry);
		instructionList.AddTail(pushInstruction);
	}
}