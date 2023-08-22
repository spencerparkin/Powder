#include "DoWhileStatementHandler.h"
#include "JumpInstruction.h"
#include "BranchInstruction.h"
#include "Assembler.h"

namespace Powder
{
	DoWhileStatementHandler::DoWhileStatementHandler()
	{
	}

	/*virtual*/ DoWhileStatementHandler::~DoWhileStatementHandler()
	{
	}

	/*virtual*/ void DoWhileStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 4 && syntaxNode->childList.GetCount() != 5)
			throw new CompileTimeException("Expected \"do-while-statement\" in AST to have exactly 4 or 5 children.", &syntaxNode->fileLocation);

		AssemblyData::Entry entry;

		// Lay down the first half of the loop instructions.
		LinkedList<Instruction*> initialLoopInstructionList;
		instructionGenerator->GenerateInstructionListRecursively(initialLoopInstructionList, syntaxNode->GetChild(1));
		instructionList.Append(initialLoopInstructionList);

		// Now lay down the conditional instructions of the loop.  What should remain is a single value on the eval stack for our branch instruction.
		LinkedList<Instruction*> conditionalInstructionList;
		instructionGenerator->GenerateInstructionListRecursively(conditionalInstructionList, syntaxNode->GetChild(3));
		instructionList.Append(conditionalInstructionList);

		// Condition failure means we jump; success, we fall through.
		BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(branchInstruction);

		// Now lay down the last half of the loop instructions, if given.
		LinkedList<Instruction*> finalLoopInstructionList;
		if (syntaxNode->childList.GetCount() == 5)
		{
			instructionGenerator->GenerateInstructionListRecursively(finalLoopInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->GetNext()->GetNext()->value);
			instructionList.Append(finalLoopInstructionList);
		}

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
	}
}