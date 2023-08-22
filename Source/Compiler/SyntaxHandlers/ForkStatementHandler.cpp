#include "ForkStatementHandler.h"
#include "ForkInstruction.h"
#include "JumpInstruction.h"
#include "Assembler.h"

namespace Powder
{
	ForkStatementHandler::ForkStatementHandler()
	{
	}

	/*virtual*/ ForkStatementHandler::~ForkStatementHandler()
	{
	}

	/*virtual*/ void ForkStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 2 && syntaxNode->childList.GetCount() != 4)
			throw new CompileTimeException("Expected \"fork-statement\" in AST to have exactly 2 or 4 children.", &syntaxNode->fileLocation);

		AssemblyData::Entry entry;

		// A fork is like an unconditional jump, but it both does and doesn't jump.
		ForkInstruction* forkInstruction = Instruction::CreateForAssembly<ForkInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(forkInstruction);

		LinkedList<Instruction*> forkedInstructionList;
		instructionGenerator->GenerateInstructionListRecursively(forkedInstructionList, syntaxNode->GetChild(1));
		instructionList.Append(forkedInstructionList);

		if (syntaxNode->childList.GetCount() == 2)
		{
			entry.Reset();
			entry.jumpDelta = forkedInstructionList.GetCount() + 1;
			entry.string = "fork";
			forkInstruction->assemblyData->configMap.Insert("jump-delta", entry);
		}
		else if (syntaxNode->childList.GetCount() == 4)
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
			instructionGenerator->GenerateInstructionListRecursively(elseInstructionList, syntaxNode->GetChild(3));
			instructionList.Append(elseInstructionList);

			entry.Reset();
			entry.jumpDelta = elseInstructionList.GetCount() + 1;
			entry.string = "jump";
			jumpInstruction->assemblyData->configMap.Insert("jump-delta", entry);
		}
	}
}