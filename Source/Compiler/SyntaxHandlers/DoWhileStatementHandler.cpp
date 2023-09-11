#include "DoWhileStatementHandler.h"
#include "JumpInstruction.h"
#include "BranchInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	DoWhileStatementHandler::DoWhileStatementHandler()
	{
	}

	/*virtual*/ DoWhileStatementHandler::~DoWhileStatementHandler()
	{
	}

	/*virtual*/ bool DoWhileStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 4 && syntaxNode->GetChildCount() != 5)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"do-while-statement\" in AST to have exactly 4 or 5 children.");
			return false;
		}

		AssemblyData::Entry entry;

		// Lay down the first half of the loop instructions.
		LinkedList<Instruction*> initialLoopInstructionList;
		if (!instructionGenerator->GenerateInstructionListRecursively(initialLoopInstructionList, syntaxNode->GetChild(1), error))
		{
			DeleteList<Instruction*>(initialLoopInstructionList);
			error.Add(std::string(syntaxNode->GetChild(1)->fileLocation) + "Failed to generate instructions for first half of loop.");
			return false;
		}
		instructionList.Append(initialLoopInstructionList);

		// Soak up any applicable break and continue jumps from the first half of the loop body.
		LinkedList<Instruction*> breakInstructionList, continueInstructionList;
		this->FindBreakAndContinueJumps(syntaxNode, initialLoopInstructionList, breakInstructionList, continueInstructionList);

		// Now lay down the conditional instructions of the loop.  What should remain is a single value on the eval stack for our branch instruction.
		LinkedList<Instruction*> conditionalInstructionList;
		if (!instructionGenerator->GenerateInstructionListRecursively(conditionalInstructionList, syntaxNode->GetChild(3), error))
		{
			DeleteList<Instruction*>(conditionalInstructionList);
			error.Add(std::string(syntaxNode->GetChild(3)->fileLocation) + "Failed to generate conditional instructions for loop.");
			return false;
		}
		instructionList.Append(conditionalInstructionList);

		// Condition failure means we jump; success, we fall through.
		BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(branchInstruction);

		// Now lay down the last half of the loop instructions, if given.
		LinkedList<Instruction*> finalLoopInstructionList;
		if (syntaxNode->GetChildCount() == 5)
		{
			if (!instructionGenerator->GenerateInstructionListRecursively(finalLoopInstructionList, syntaxNode->GetChild(4), error))
			{
				DeleteList<Instruction*>(finalLoopInstructionList);
				error.Add(std::string(syntaxNode->GetChild(4)->fileLocation) + "Failed to generate instructions for last half of loop.");
				return false;
			}
			instructionList.Append(finalLoopInstructionList);
		}

		// Add more break and continue jumps from the last half of the loop body.
		this->FindBreakAndContinueJumps(syntaxNode, finalLoopInstructionList, breakInstructionList, continueInstructionList);

		// Unconditionally jump back up to the top of the do-while-loop.
		JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(syntaxNode->fileLocation);
		entry.Reset();
		entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
		jumpInstruction->assemblyData->configMap.Insert("type", entry);
		entry.instruction = initialLoopInstructionList.GetHead()->value;
		jumpInstruction->assemblyData->configMap.Insert("jump", entry);
		instructionList.AddTail(jumpInstruction);

		// We now know enough to resolve the branch jump delta.
		entry.Reset();
		entry.jumpDelta = finalLoopInstructionList.GetCount() + 2;
		entry.string = "branch";
		branchInstruction->assemblyData->configMap.Insert("jump-delta", entry);

		// Break-statement jumps should jump to the same place.
		for (LinkedList<Instruction*>::Node* node = breakInstructionList.GetHead(); node; node = node->GetNext())
		{
			Instruction* jumpInstruction = node->value;
			entry.Reset();
			entry.instruction = branchInstruction;
			entry.string = "jump";
			jumpInstruction->assemblyData->configMap.Insert("copy-cat-jump", entry);
		}

		// Continue-statement jumps go to the conditional part of the loop construct.
		for (LinkedList<Instruction*>::Node* node = continueInstructionList.GetHead(); node; node = node->GetNext())
		{
			Instruction* jumpInstruction = node->value;
			entry.Reset();
			entry.instruction = conditionalInstructionList.GetHead()->value;
			jumpInstruction->assemblyData->configMap.Insert("jump", entry);
		}

		return true;
	}
}