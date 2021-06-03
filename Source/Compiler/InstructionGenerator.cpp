#include "InstructionGenerator.h"
#include "Assembler.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "Exceptions.hpp"

namespace Powder
{
	InstructionGenerator::InstructionGenerator()
	{
	}

	/*virtual*/ InstructionGenerator::~InstructionGenerator()
	{
	}

	void InstructionGenerator::GenerateInstructionList(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode)
	{
		AssemblyData::Entry entry;

		if (*syntaxNode->name == "statement-list")
		{
			for (const LinkedList<Parser::SyntaxNode*>::Node* node = syntaxNode->childList.GetHead(); node; node = node->GetNext())
			{
				if (*node->value->name != "statement")
					throw new CompileTimeException("Expected \"statement\" under \"statement-list\" in AST");

				// We simply execute the code for each statement in order.
				this->GenerateInstructionList(instructionList, node->value);

				// TODO: Patch jump pointers here as necessary.  E.g., resolve all jump hints in the assembly data of the instructions.
			}
		}
		else if (*syntaxNode->name == "statement")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"statement\" in AST to have exactly one child.");

			this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->value);
		}
		else if (*syntaxNode->name == "if-statement")
		{
			if (syntaxNode->childList.GetCount() != 3 && syntaxNode->childList.GetCount() != 5)
				throw new CompileTimeException("Expected \"if-statement\" in AST to have exactly 3 or 5 children.");

			// Execute conditional instructions.  What remains on the evaluation stack top gets consumed by the branch instruction.
			this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->GetNext()->value);
			
			// The branch instruction falls through if the condition passes, and jumps if the condition fails.
			BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>();
			instructionList.AddTail(branchInstruction);
			
			// Lay down condition-pass instructions.
			LinkedList<Instruction*> passInstructionList;
			this->GenerateInstructionList(passInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);
			instructionList.Append(passInstructionList);

			// Else clause?
			if (syntaxNode->childList.GetCount() != 5)
			{
				// No.  Setup jump-hint on the branch instruction to jump to instruction just after the last condition-pass instruction.
				entry.jumpDelta = passInstructionList.GetCount();
				branchInstruction->assemblyData->configMap.Insert("jump-hint", entry);
			}
			else
			{
				// Yes.  Before laying down the condition-fail instructions, we want an unconditional jump that goes over them if the condition passed.
				JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
				instructionList.AddTail(jumpInstruction);

				// Okay, now lay down the condition-fail instructions.
				LinkedList<Instruction*> failInstructionList;
				this->GenerateInstructionList(failInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->GetNext()->GetNext()->value);
				instructionList.Append(failInstructionList);
				entry.jumpDelta = failInstructionList.GetCount();
				jumpInstruction->assemblyData->configMap.Insert("jump-hint", entry);

				// We have enough now to resolve the conditional jump instruction.
				entry.instruction = failInstructionList.GetHead()->value;
				branchInstruction->assemblyData->configMap.Insert("branch", entry);
			}
		}
	}
}