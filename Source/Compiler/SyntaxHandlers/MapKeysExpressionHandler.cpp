#include "MapKeysExpressionHandler.h"
#include "MapInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	MapKeysExpressionHandler::MapKeysExpressionHandler()
	{
	}

	/*virtual*/ MapKeysExpressionHandler::~MapKeysExpressionHandler()
	{
	}

	/*virtual*/ bool MapKeysExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 2)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"map-keys-expression\" in AST to have exactly 2 children.");
			return false;
		}

		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(1), error))
		{
			error.Add(std::string(syntaxNode->GetChild(1)->fileLocation) + "Failed to generate instructions for leaving map value on eval stack top.");
			return false;
		}

		MapInstruction* mapInstruction = Instruction::CreateForAssembly<MapInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MapInstruction::Action::MAKE_KEY_LIST;
		mapInstruction->assemblyData->configMap.Insert("action", entry);
		instructionList.AddTail(mapInstruction);

		return true;
	}
}