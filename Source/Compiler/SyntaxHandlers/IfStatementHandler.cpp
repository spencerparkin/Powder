#include "IfStatementHandler.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	IfStatementHandler::IfStatementHandler()
	{
	}

	/*virtual*/ IfStatementHandler::~IfStatementHandler()
	{
	}

	/*virtual*/ bool IfStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 3 && syntaxNode->GetChildCount() != 5)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"if-statement\" in AST to have exactly 3 or 5 children.");
			return false;
		}

		AssemblyData::Entry entry;

		// Execute conditional instructions.  What remains on the evaluation stack top gets consumed by the branch instruction.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(1), error))
		{
			error.Add(std::string(syntaxNode->GetChild(1)->fileLocation) + "Failed to generate instructions for if-statement conditional.");
			return false;
		}

		// The branch instruction falls through if the condition passes, and jumps if the condition fails.
		BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(branchInstruction);

		// Lay down condition-pass instructions.
		LinkedList<Instruction*> passInstructionList;
		if (!instructionGenerator->GenerateInstructionListRecursively(passInstructionList, syntaxNode->GetChild(2), error))
		{
			DeleteList<Instruction*>(passInstructionList);
			error.Add(std::string(syntaxNode->GetChild(2)->fileLocation) + "Failed to generate instructions pass-condition body.");
			return false;
		}
		instructionList.Append(passInstructionList);

		// Else clause?
		if (syntaxNode->GetChildCount() != 5)
		{
			// No.  Setup jump-hint on the branch instruction to jump to instruction just after the last condition-pass instruction.
			entry.Reset();
			entry.jumpDelta = passInstructionList.GetCount() + 1;
			entry.string = "branch";
			branchInstruction->assemblyData->configMap.Insert("jump-delta", entry);
		}
		else
		{
			// Yes.  Before laying down the condition-fail instructions, we want an unconditional jump that goes over them if the condition passed.
			JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(syntaxNode->fileLocation);
			entry.Reset();
			entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
			jumpInstruction->assemblyData->configMap.Insert("type", entry);
			instructionList.AddTail(jumpInstruction);

			// Okay, now lay down the condition-fail instructions.
			LinkedList<Instruction*> failInstructionList;
			if (!instructionGenerator->GenerateInstructionListRecursively(failInstructionList, syntaxNode->GetChild(4), error))
			{
				DeleteList<Instruction*>(failInstructionList);
				error.Add(std::string(syntaxNode->GetChild(4)->fileLocation) + "Failed to generate instructions fail-condition body.");
				return false;
			}
			instructionList.Append(failInstructionList);

			// We have enough now to resolve the jump-delta for getting over the else-clause.
			entry.Reset();
			entry.jumpDelta = failInstructionList.GetCount() + 1;
			entry.string = "jump";
			jumpInstruction->assemblyData->configMap.Insert("jump-delta", entry);

			// We have enough now to resolve the conditional jump instruction.
			entry.Reset();
			entry.instruction = failInstructionList.GetHead()->value;
			branchInstruction->assemblyData->configMap.Insert("branch", entry);
		}

		return true;
	}
}