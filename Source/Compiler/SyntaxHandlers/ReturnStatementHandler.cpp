#include "ReturnStatementHandler.h"
#include "LoadInstruction.h"
#include "JumpInstruction.h"
#include "PushInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	ReturnStatementHandler::ReturnStatementHandler()
	{
	}

	/*virtual*/ ReturnStatementHandler::~ReturnStatementHandler()
	{
	}

	/*virtual*/ bool ReturnStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		const ParseParty::Parser::SyntaxNode* funcDefNode = syntaxNode->GetParent();
		while (funcDefNode)
		{
			if (*funcDefNode->text == "function-definition")
				break;

			funcDefNode = funcDefNode->GetParent();
		}

		if (!funcDefNode)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Return statement issued not in the context of a function definition.");
			return false;
		}

		if (syntaxNode->GetChildCount() == 2)
		{
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(1), error))
			{
				error.Add(std::string(syntaxNode->GetChild(1)->fileLocation) + "Failed to generate instructions resulting in return value left on eval stack top.");
				return false;
			}
		}
		else
		{
			PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
			AssemblyData::Entry entry;
			entry.code = PushInstruction::DataType::NULL_VALUE;
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

		return true;
	}
}