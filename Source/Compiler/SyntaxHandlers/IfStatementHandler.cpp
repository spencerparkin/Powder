#include "IfStatementHandler.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "Assembler.h"

namespace Powder
{
	IfStatementHandler::IfStatementHandler()
	{
	}

	/*virtual*/ IfStatementHandler::~IfStatementHandler()
	{
	}

	/*virtual*/ void IfStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 3 && syntaxNode->childList.GetCount() != 5)
			throw new CompileTimeException("Expected \"if-statement\" in AST to have exactly 3 or 5 children.", &syntaxNode->fileLocation);

		AssemblyData::Entry entry;

		// Execute conditional instructions.  What remains on the evaluation stack top gets consumed by the branch instruction.
		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->value);

		// The branch instruction falls through if the condition passes, and jumps if the condition fails.
		BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(branchInstruction);

		// Lay down condition-pass instructions.
		LinkedList<Instruction*> passInstructionList;
		instructionGenerator->GenerateInstructionListRecursively(passInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);
		instructionList.Append(passInstructionList);

		// Else clause?
		if (syntaxNode->childList.GetCount() != 5)
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
			instructionGenerator->GenerateInstructionListRecursively(failInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->GetNext()->GetNext()->value);
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
	}
}