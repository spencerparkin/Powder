#include "TernaryExpressionHandler.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "Error.h"

using namespace Powder;

TernaryExpressionHandler::TernaryExpressionHandler()
{
}

/*virtual*/ TernaryExpressionHandler::~TernaryExpressionHandler()
{
}

/*virtual*/ bool TernaryExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
{
	if (syntaxNode->GetChildCount() != 5)
	{
		error.Add(std::string(syntaxNode->fileLocation) + "Expected ternary expression node to have 5 children.");
		return false;
	}

	const ParseParty::Parser::SyntaxNode* conditionNode = syntaxNode->GetChild(0);
	const ParseParty::Parser::SyntaxNode* conditionPassNode = syntaxNode->GetChild(2);
	const ParseParty::Parser::SyntaxNode* conditionFailNode = syntaxNode->GetChild(4);

	if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, conditionNode, error))
	{
		error.Add(std::string(conditionNode->fileLocation) + "Failed to generate instructions for condition of ternary expression.");
		return false;
	}

	BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>(conditionNode->fileLocation);
	instructionList.AddTail(branchInstruction);

	if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, conditionPassNode, error))
	{
		error.Add(std::string(conditionPassNode->fileLocation) + "Failed to generate instructions for condition-pass clause of ternary expression.");
		return false;
	}

	JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(conditionPassNode->fileLocation);
	AssemblyData::Entry entry;
	entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
	jumpInstruction->assemblyData->configMap.Insert("type", entry);
	instructionList.AddTail(jumpInstruction);
	int64_t i = instructionList.GetCount();
	LinkedList<Instruction*>::Node* node = instructionList.GetTail();

	if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, conditionFailNode, error))
	{
		error.Add(std::string(conditionPassNode->fileLocation) + "Failed to generate instructions for condition-fail clause of ternary expression.");
		return false;
	}

	int64_t j = instructionList.GetCount();
	entry.Reset();
	entry.jumpDelta = j - i + 1;
	entry.string = "jump";
	jumpInstruction->assemblyData->configMap.Insert("jump-delta", entry);

	entry.Reset();
	entry.instruction = node->GetNext()->value;
	branchInstruction->assemblyData->configMap.Insert("branch", entry);

	return true;
}