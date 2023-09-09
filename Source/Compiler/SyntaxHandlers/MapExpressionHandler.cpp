#include "MapExpressionHandler.h"
#include "MapInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	MapExpressionHandler::MapExpressionHandler()
	{
	}

	/*virtual*/ MapExpressionHandler::~MapExpressionHandler()
	{
	}

	/*virtual*/ bool MapExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		// The map may be empty, in which case, we don't find the node.
		const ParseParty::Parser::SyntaxNode* mapPairListNode = syntaxNode->FindChild("map-pair-list", 1);
		if (mapPairListNode)
		{
			for (const ParseParty::Parser::SyntaxNode* mapPairNode : *mapPairListNode->childList)
			{
				if (*mapPairNode->text != "map-pair")
				{
					error.Add(std::string(mapPairNode->fileLocation) + "Expected all children of \"map-pair-list\" to be \"map-pair\" in AST.");
					return false;
				}

				if (mapPairNode->GetChildCount() != 3)
				{
					error.Add(std::string(mapPairNode->fileLocation) + "Expected \"map-pair\" node in AST to have exactly 3 children.");
					return false;
				}

				// Push the field value.
				if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, mapPairNode->GetChild(0), error))
				{
					error.Add(std::string(mapPairNode->GetChild(0)->fileLocation) + "Failed to generate instructions to push field value.");
					return false;
				}

				// Push the data value.
				if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, mapPairNode->GetChild(2), error))
				{
					error.Add(std::string(mapPairNode->GetChild(2)->fileLocation) + "Failed to generate instructions to push data value.");
					return false;
				}

				// Now insert the data value at the field value.  Field and data values are popped; the map value remains on the stack top.
				MapInstruction* mapInstruction = Instruction::CreateForAssembly<MapInstruction>(syntaxNode->fileLocation);
				AssemblyData::Entry entry;
				entry.code = MapInstruction::Action::INSERT;
				mapInstruction->assemblyData->configMap.Insert("action", entry);
				instructionList.AddTail(mapInstruction);
			}
		}

		return true;
	}
}