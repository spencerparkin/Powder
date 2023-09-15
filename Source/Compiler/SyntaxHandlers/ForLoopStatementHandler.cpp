#include "ForLoopStatementHandler.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "Error.h"

using namespace Powder;

ForLoopStatementHandler::ForLoopStatementHandler()
{
}

/*virtual*/ ForLoopStatementHandler::~ForLoopStatementHandler()
{
}

/*virtual*/ bool ForLoopStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
{
	if (syntaxNode->GetChildCount() != 3)
	{
		error.Add(std::string(syntaxNode->fileLocation) + "Expected \"for-loop-statement\" to have 3 children.");
		return false;
	}

	const ParseParty::Parser::SyntaxNode* logicNode = syntaxNode->GetChild(1);
	const ParseParty::Parser::SyntaxNode* forLoopBodyNode = syntaxNode->GetChild(2);

	if (*logicNode->text != "for-loop-logic")
	{
		error.Add(std::string(logicNode->fileLocation) + "Expected second child of \"for-loop-statement\" to be a \"for-loop-logic\" node.");
		return false;
	}
	
	if (logicNode->GetChildCount() != 3)
	{
		error.Add(std::string(logicNode->fileLocation) + "Expected \"for-loop-logic\" node to have exactly 3 children.");
		return false;
	}

	const ParseParty::Parser::SyntaxNode* forInitNode = logicNode->GetChild(0);
	const ParseParty::Parser::SyntaxNode* forConditionNode = logicNode->GetChild(1);
	const ParseParty::Parser::SyntaxNode* forAdvanceNode = logicNode->GetChild(2);

	if (forInitNode->GetChildCount() != 1)
	{
		error.Add(std::string(forInitNode->fileLocation) + "Expected \"for-init-statement\" to have a single child.");
		return false;
	}

	if (forConditionNode->GetChildCount() != 1)
	{
		error.Add(std::string(forConditionNode->fileLocation) + "Expected \"for-condition-statement\" to have a single child.");
		return false;
	}

	if (forAdvanceNode->GetChildCount() != 1)
	{
		error.Add(std::string(forAdvanceNode->fileLocation) + "Expected \"for-advance-statement\" to have a single child.");
		return false;
	}

	// First thing's first.  Generate the initialization instructions.
	if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, forInitNode->GetChild(0), error))
	{
		error.Add(std::string(forInitNode->fileLocation) + "Failed to generate instructions for for-loop initialization statement.");
		return false;
	}

	// The conditional should, as a net result, push a value onto the eval stack.
	LinkedList<Instruction*> conditionalInstructionsList;
	if (!instructionGenerator->GenerateInstructionListRecursively(conditionalInstructionsList, forConditionNode->GetChild(0), error))
	{
		DeleteList<Instruction*>(conditionalInstructionsList);
		error.Add(std::string(forConditionNode->fileLocation) + "Failed to generate instructions for for-loop conditional statement.");
		return false;
	}
	instructionList.Append(conditionalInstructionsList);

	// We then branch out of the for-loop if the condition fails.  We don't yet know where to jump, so just add the instruction for now.
	BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>(forConditionNode->fileLocation);
	instructionList.AddTail(branchInstruction);

	// Next comes the for-loop body instructions.  Lay them down now.
	LinkedList<Instruction*> forBodyInstructionList;
	if (!instructionGenerator->GenerateInstructionListRecursively(forBodyInstructionList, forLoopBodyNode, error))
	{
		DeleteList<Instruction*>(forBodyInstructionList);
		error.Add(std::string(forLoopBodyNode->fileLocation) + "Failed to generate instructions for for-loop body.");
		return false;
	}
	instructionList.Append(forBodyInstructionList);

	// Soak up any immediately nested break and continue statements so that we can patch them appropriately.
	LinkedList<Instruction*> breakInstructionList, continueInstructionList;
	this->FindBreakAndContinueJumps(syntaxNode, forBodyInstructionList, breakInstructionList, continueInstructionList);

	// An iteration for the loop is complete.  Time to advance.  Do that now.
	LinkedList<Instruction*> advanceInstructionList;
	if (!instructionGenerator->GenerateInstructionListRecursively(advanceInstructionList, forAdvanceNode->GetChild(0), error))
	{
		DeleteList<Instruction*>(advanceInstructionList);
		error.Add(std::string(forAdvanceNode->fileLocation) + "Failed to generate instructions for for-loop advance statement.");
		return false;
	}
	instructionList.Append(advanceInstructionList);

	// We now unconditionally jump back up to the conditional part of the for-loop.
	JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(forAdvanceNode->fileLocation);
	AssemblyData::Entry entry;
	entry.instruction = conditionalInstructionsList.GetHead()->value;
	jumpInstruction->assemblyData->configMap.Insert("jump", entry);
	entry.Reset();
	entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
	jumpInstruction->assemblyData->configMap.Insert("type", entry);
	instructionList.AddTail(jumpInstruction);

	// We now know enough to patch the for-loop conditional branch.
	entry.Reset();
	entry.string = "branch";
	entry.jumpDelta = forBodyInstructionList.GetCount() + advanceInstructionList.GetCount() + 2;
	branchInstruction->assemblyData->configMap.Insert("jump-delta", entry);

	// The break-statement jumps should jump to the same place, wherever that will happen to be.
	for (LinkedList<Instruction*>::Node* node = breakInstructionList.GetHead(); node; node = node->GetNext())
	{
		Instruction* jumpInstruction = node->value;
		entry.Reset();
		entry.instruction = branchInstruction;
		entry.string = "jump";
		jumpInstruction->assemblyData->configMap.Insert("copy-cat-jump", entry);
	}

	// The continue-statement jumps go to the advancement part of the for-loop.
	for (LinkedList<Instruction*>::Node* node = continueInstructionList.GetHead(); node; node = node->GetNext())
	{
		Instruction* jumpInstruction = node->value;
		entry.Reset();
		entry.instruction = advanceInstructionList.GetHead()->value;
		jumpInstruction->assemblyData->configMap.Insert("jump", entry);
	}

	return true;
}