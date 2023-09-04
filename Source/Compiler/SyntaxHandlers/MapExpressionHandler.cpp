#include "MapExpressionHandler.h"
#include "MapInstruction.h"
#include "Assembler.h"

namespace Powder
{
	MapExpressionHandler::MapExpressionHandler()
	{
	}

	/*virtual*/ MapExpressionHandler::~MapExpressionHandler()
	{
	}

	/*virtual*/ void MapExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		// The map may be empty, in which case, we don't find the node.
		const ParseParty::Parser::SyntaxNode* mapPairListNode = syntaxNode->FindChild("map-pair-list", 1);
		if (mapPairListNode)
		{
			for (const ParseParty::Parser::SyntaxNode* mapPairNode : *mapPairListNode->childList)
			{
				if (*mapPairNode->text != "map-pair")
					throw new CompileTimeException("Expected all children of \"map-pair-list\" to be \"map-pair\" in AST.", &mapPairNode->fileLocation);

				if (mapPairNode->GetChildCount() != 3)
					throw new CompileTimeException("Expected \"map-pair\" node in AST to have exactly 3 children.", &mapPairNode->fileLocation);

				// Push the field value.
				instructionGenerator->GenerateInstructionListRecursively(instructionList, mapPairNode->GetChild(0));

				// Push the data value.
				instructionGenerator->GenerateInstructionListRecursively(instructionList, mapPairNode->GetChild(2));

				// Now insert the data value at the field value.  Field and data values are popped; the map value remains on the stack top.
				MapInstruction* mapInstruction = Instruction::CreateForAssembly<MapInstruction>(syntaxNode->fileLocation);
				AssemblyData::Entry entry;
				entry.code = MapInstruction::Action::INSERT;
				mapInstruction->assemblyData->configMap.Insert("action", entry);
				instructionList.AddTail(mapInstruction);
			}
		}
	}
}