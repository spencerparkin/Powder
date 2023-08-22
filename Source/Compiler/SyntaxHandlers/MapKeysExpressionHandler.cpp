#include "MapKeysExpressionHandler.h"
#include "MapInstruction.h"
#include "Assembler.h"

namespace Powder
{
	MapKeysExpressionHandler::MapKeysExpressionHandler()
	{
	}

	/*virtual*/ MapKeysExpressionHandler::~MapKeysExpressionHandler()
	{
	}

	/*virtual*/ void MapKeysExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 2)
			throw new CompileTimeException("Expected \"map-keys-expression\" in AST to have exactly 2 children.", &syntaxNode->fileLocation);

		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(1));

		MapInstruction* mapInstruction = Instruction::CreateForAssembly<MapInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MapInstruction::Action::MAKE_KEY_LIST;
		mapInstruction->assemblyData->configMap.Insert("action", entry);
		instructionList.AddTail(mapInstruction);
	}
}