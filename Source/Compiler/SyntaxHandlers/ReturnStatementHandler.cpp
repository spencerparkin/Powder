#include "ReturnStatementHandler.h"
#include "LoadInstruction.h"
#include "JumpInstruction.h"
#include "PushInstruction.h"
#include "Assembler.h"

namespace Powder
{
	ReturnStatementHandler::ReturnStatementHandler()
	{
	}

	/*virtual*/ ReturnStatementHandler::~ReturnStatementHandler()
	{
	}

	/*virtual*/ void ReturnStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() == 2)
			instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(1));
		else
		{
			PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
			AssemblyData::Entry entry;
			entry.code = PushInstruction::DataType::UNDEFINED;
			pushInstruction->assemblyData->configMap.Insert("type", entry);
			instructionList.AddTail(pushInstruction);
		}

		LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.string = "__return_address__";
		loadInstruction->assemblyData->configMap.Insert("name", entry);
		instructionList.AddTail(loadInstruction);

		JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(syntaxNode->fileLocation);
		entry.Reset();
		entry.code = JumpInstruction::JUMP_TO_LOADED_ADDRESS;
		jumpInstruction->assemblyData->configMap.Insert("type", entry);
		instructionList.AddTail(jumpInstruction);
	}
}