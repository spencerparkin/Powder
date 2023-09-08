#include "ForkStatementHandler.h"
#include "ForkInstruction.h"
#include "JumpInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	ForkStatementHandler::ForkStatementHandler()
	{
	}

	/*virtual*/ ForkStatementHandler::~ForkStatementHandler()
	{
	}

	/*virtual*/ bool ForkStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 2 && syntaxNode->GetChildCount() != 4)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"fork-statement\" in AST to have exactly 2 or 4 children.");
			return false;
		}

		AssemblyData::Entry entry;

		// A fork is like an unconditional jump, but it both does and doesn't jump.
		ForkInstruction* forkInstruction = Instruction::CreateForAssembly<ForkInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(forkInstruction);

		LinkedList<Instruction*> forkedInstructionList;
		if (!instructionGenerator->GenerateInstructionListRecursively(forkedInstructionList, syntaxNode->GetChild(1), error))
		{
			DeleteList<Instruction*>(forkedInstructionList);
			error.Add(std::string(syntaxNode->fileLocation) + "Failed to generate forked instructions.");
			return false;
		}
		instructionList.Append(forkedInstructionList);

		if (syntaxNode->GetChildCount() == 2)
		{
			entry.Reset();
			entry.jumpDelta = forkedInstructionList.GetCount() + 1;
			entry.string = "fork";
			forkInstruction->assemblyData->configMap.Insert("jump-delta", entry);
		}
		else if (syntaxNode->GetChildCount() == 4)
		{
			entry.Reset();
			entry.jumpDelta = forkedInstructionList.GetCount() + 2;
			entry.string = "fork";
			forkInstruction->assemblyData->configMap.Insert("jump-delta", entry);

			JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(syntaxNode->fileLocation);
			entry.Reset();
			entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
			jumpInstruction->assemblyData->configMap.Insert("type", entry);
			instructionList.AddTail(jumpInstruction);

			LinkedList<Instruction*> elseInstructionList;
			if (!instructionGenerator->GenerateInstructionListRecursively(elseInstructionList, syntaxNode->GetChild(3), error))
			{
				DeleteList<Instruction*>(elseInstructionList);
				error.Add(std::string(syntaxNode->GetChild(3)->fileLocation) + "Failed to generate else-clause of fork instruction.");
				return false;
			}
			instructionList.Append(elseInstructionList);

			entry.Reset();
			entry.jumpDelta = elseInstructionList.GetCount() + 1;
			entry.string = "jump";
			jumpInstruction->assemblyData->configMap.Insert("jump-delta", entry);
		}

		return true;
	}
}