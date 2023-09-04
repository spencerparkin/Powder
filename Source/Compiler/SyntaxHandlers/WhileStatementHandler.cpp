#include "WhileStatementHandler.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "Assembler.h"

namespace Powder
{
	WhileStatementHandler::WhileStatementHandler()
	{
	}

	/*virtual*/ WhileStatementHandler::~WhileStatementHandler()
	{
	}

	/*virtual*/ void WhileStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->GetChildCount() != 3)
			throw new CompileTimeException("Expected \"while-statement\" in AST to have exactly 3 children.", &syntaxNode->fileLocation);

		AssemblyData::Entry entry;

		// Lay down the conditional instructions first.  What remains is a value on the eval stack who's truthiness we'll use in a branch instruction.
		LinkedList<Instruction*> conditionalInstructionList;
		instructionGenerator->GenerateInstructionListRecursively(conditionalInstructionList, syntaxNode->GetChild(1));
		instructionList.Append(conditionalInstructionList);

		// The branch falls through if the bool is true, or jumps in the bool is false.  We don't yet know how far to jump to get over the while-loop body.
		BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(branchInstruction);

		// Lay down while-loop body instructions.
		LinkedList<Instruction*> whileLoopBodyInstructionList;
		instructionGenerator->GenerateInstructionListRecursively(whileLoopBodyInstructionList, syntaxNode->GetChild(2));
		instructionList.Append(whileLoopBodyInstructionList);

		// Unconditionally jump back to the top of the while-loop where the conditional is evaluated.
		JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(syntaxNode->fileLocation);
		entry.Reset();
		entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
		jumpInstruction->assemblyData->configMap.Insert("type", entry);
		entry.instruction = conditionalInstructionList.GetHead()->value;
		jumpInstruction->assemblyData->configMap.Insert("jump", entry);
		instructionList.AddTail(jumpInstruction);

		// We now know enough to resolve the branch jump delta.  It's the size of the body plus the unconditional jump.
		entry.Reset();
		entry.jumpDelta = whileLoopBodyInstructionList.GetCount() + 2;
		entry.string = "branch";
		branchInstruction->assemblyData->configMap.Insert("jump-delta", entry);
	}
}