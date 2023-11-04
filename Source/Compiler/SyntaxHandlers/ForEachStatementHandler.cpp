#include "ForEachStatementHandler.h"
#include "FunctionCallExpressionHandler.h"
#include "PushInstruction.h"
#include "PopInstruction.h"
#include "ListInstruction.h"
#include "StoreInstruction.h"
#include "LoadInstruction.h"
#include "BranchInstruction.h"
#include "MathInstruction.h"
#include "JumpInstruction.h"
#include "SysCallInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	ForEachStatementHandler::ForEachStatementHandler()
	{
	}

	/*virtual*/ ForEachStatementHandler::~ForEachStatementHandler()
	{
	}

	/*virtual*/ bool ForEachStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 3)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"for-statement\" in AST to have exactly 3 children.");
			return false;
		}

		const ParseParty::Parser::SyntaxNode* iterationNode = syntaxNode->GetChild(1);
		if (*iterationNode->text != "for-iteration-expression")
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"for-statement\" in AST to have \"for-iteration-expression\" child.");
			return false;
		}

		if (iterationNode->GetChildCount() != 3)
		{
			error.Add(std::string(iterationNode->fileLocation) + "Exected \"for-iteration-expression\" in AST to have exactly 3 children.");
			return false;
		}

		const ParseParty::Parser::SyntaxNode* identifierNode = iterationNode->GetChild(0);
		if (*identifierNode->text != "@identifier")
		{
			error.Add(std::string(identifierNode->fileLocation) + "Expected \"@identifier\" in AST as child of \"for-iteration-expression\" node.");
			return false;
		}

		// Push the container value or iterator function onto the eval-stack.
		const ParseParty::Parser::SyntaxNode* iteratorNode = iterationNode->GetChild(2);
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, iteratorNode, error))
		{
			error.Add(std::string(iteratorNode->fileLocation) + "Failed to generate instructions to push container value or iterator function onto the eval stack.");
			return false;
		}

		// Convert it to an iterator if necessary.
		SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>(iteratorNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = SysCallInstruction::AS_ITERATOR;
		sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
		instructionList.AddTail(sysCallInstruction);

		// Push an argument list onto the eval stack, then push the iterator function address on top of that.
		if (!this->GenerateInstructionForIteratorCallSetup(instructionList, "reset", iterationNode->fileLocation, error))
		{
			error.Add(std::string(iterationNode->fileLocation) + "Failed to generate instruction to setup iterator call.");
			return false;
		}

		// We're now ready to make the call to the iterator to reset iteration (to set it up for the first time, really.)
		if (!FunctionCallExpressionHandler::GenerateCallInstructions(instructionList, iterationNode->fileLocation, error))
		{
			error.Add(std::string(iterationNode->fileLocation) + "Failed to geneate instruction to make function call.");
			return false;
		}

		// The result of resetting iteration is always a value we throw away, so do that now.
		PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(iterationNode->fileLocation);
		instructionList.AddTail(popInstruction);

		// We're now ready to begin the loop instructions.  Call the iterator to generate the first/next value.
		LinkedList<Instruction*> forLoopHeadInstructionList;
		if (!this->GenerateInstructionForIteratorCallSetup(forLoopHeadInstructionList, "next", iterationNode->fileLocation, error))
		{
			DeleteList<Instruction*>(forLoopHeadInstructionList);
			error.Add(std::string(iterationNode->fileLocation) + "Failed to generate instruction to setup iterator call.");
			return false;
		}
		
		if (!FunctionCallExpressionHandler::GenerateCallInstructions(forLoopHeadInstructionList, iterationNode->fileLocation, error))
		{
			DeleteList<Instruction*>(forLoopHeadInstructionList);
			error.Add(std::string(iterationNode->fileLocation) + "Failed to generate instructions to make iterator call.");
			return false;
		}

		// Store the returned value in the loop iteration variable.
		StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(iterationNode->fileLocation);
		entry.Reset();
		entry.string = *identifierNode->GetChild(0)->text;
		storeInstruction->assemblyData->configMap.Insert("name", entry);
		forLoopHeadInstructionList.AddTail(storeInstruction);

		// Iteration ends when the iteration variable becomes null, so test that now.
		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(iterationNode->fileLocation);
		entry.Reset();
		entry.code = PushInstruction::NULL_VALUE;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		forLoopHeadInstructionList.AddTail(pushInstruction);
		LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>(iterationNode->fileLocation);
		entry.Reset();
		entry.string = *identifierNode->GetChild(0)->text;
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
		if (!instructionGenerator->GenerateInstructionListRecursively(forLoopBodyInstructionList, syntaxNode->GetChild(2), error))
		{
			DeleteList<Instruction*>(forLoopHeadInstructionList);
			DeleteList<Instruction*>(forLoopBodyInstructionList);
			error.Add(std::string(syntaxNode->GetChild(2)->fileLocation) + "Failed to generate for-loop body instructions");
			return false;
		}

		// Next we now look at the for-loop body instructions to see if we find any break or continue statements applicable to us.
		LinkedList<Instruction*> breakInstructionList, continueInstructionList;
		this->FindBreakAndContinueJumps(syntaxNode, forLoopBodyInstructionList, breakInstructionList, continueInstructionList);

		// Lastly, unconditionally jump back up to the top of the for-loop and do it all over again.
		JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(iterationNode->fileLocation);
		entry.Reset();
		entry.instruction = forLoopHeadInstructionList.GetHead()->value;
		jumpInstruction->assemblyData->configMap.Insert("jump", entry);
		entry.Reset();
		entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
		jumpInstruction->assemblyData->configMap.Insert("type", entry);
		forLoopBodyInstructionList.AddTail(jumpInstruction);

		// Okay, now lay down all the loop instructions, head and body.
		instructionList.Append(forLoopHeadInstructionList);
		instructionList.Append(forLoopBodyInstructionList);

		// Lastly, pop the iterator function off the eval-stack.
		popInstruction = Instruction::CreateForAssembly<PopInstruction>(iterationNode->fileLocation);
		instructionList.AddTail(popInstruction);

		// We can now patch our branch that takes us out of the for-loop.
		entry.Reset();
		entry.instruction = popInstruction;
		branchInstruction->assemblyData->configMap.Insert("branch", entry);

		// We patch the break-statement jumps the same way.
		for (LinkedList<Instruction*>::Node* node = breakInstructionList.GetHead(); node; node = node->GetNext())
		{
			AssemblyData::Entry* jumpEntry = node->value->assemblyData->configMap.LookupPtr("jump");
			jumpEntry->instruction = popInstruction;
		}

		// The continue-statement jumps go to the top of the loop.
		for (LinkedList<Instruction*>::Node* node = continueInstructionList.GetHead(); node; node = node->GetNext())
		{
			AssemblyData::Entry* jumpEntry = node->value->assemblyData->configMap.LookupPtr("jump");
			jumpEntry->instruction = forLoopHeadInstructionList.GetHead()->value;
		}

		return true;
	}

	/*static*/ bool ForEachStatementHandler::GenerateInstructionForIteratorCallSetup(LinkedList<Instruction*>& instructionList, const char* action, const ParseParty::Lexer::FileLocation& fileLocation, Error& error)
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
		entry.offset = 1;
		pushInstruction->assemblyData->configMap.Insert("data", entry);
		instructionList.AddTail(pushInstruction);

		return true;
	}
}