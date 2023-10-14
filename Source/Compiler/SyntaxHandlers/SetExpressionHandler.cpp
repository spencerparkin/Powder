#include "SetExpressionHandler.h"
#include "SysCallInstruction.h"
#include "PopInstruction.h"
#include "PushInstruction.h"
#include "Error.h"
#include "Assembler.h"

using namespace Powder;

SetExpressionHandler::SetExpressionHandler()
{
}

/*virtual*/ SetExpressionHandler::~SetExpressionHandler()
{
}

/*virtual*/ bool SetExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
{
	using namespace ParseParty;

	if (syntaxNode->GetChildCount() != 1)
	{
		error.Add(std::string(syntaxNode->fileLocation) + "Expected exactly one child node.");
		return false;
	}

	const Parser::SyntaxNode* memberListNode = syntaxNode->GetChild(0);
	if (*memberListNode->text == "set-member-list")
	{
		for (const Parser::SyntaxNode* memberNode : *memberListNode->childList)
		{
			// To make the system call below, we need to pass the set on the stack, and it's already on the
			// stack, but we need to preserve that, because we didn't push it, and the sys-call will pop it.
			PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(memberNode->fileLocation);
			AssemblyData::Entry entry;
			entry.code = PushInstruction::DataType::EXISTING_VALUE;
			pushInstruction->assemblyData->configMap.Insert("type", entry);
			entry.Reset();
			entry.offset = 0;
			pushInstruction->assemblyData->configMap.Insert("data", entry);
			instructionList.AddTail(pushInstruction);

			// Push the member value onto the stack.
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, memberNode, error))
			{
				error.Add(std::string(memberNode->fileLocation) + "Failed to generate instruction to push member value.");
				return false;
			}

			// Now make a system call to add the member to the set.
			SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>(syntaxNode->fileLocation);
			entry.Reset();
			entry.code = SysCallInstruction::SysCall::ADD_MEMBER;
			sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
			instructionList.AddTail(sysCallInstruction);

			// Lastly, pop the return value of the system call.  We ignore it.
			PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
			instructionList.AddTail(popInstruction);
		}
	}

	return true;
}