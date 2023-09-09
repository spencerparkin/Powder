#include "ListExpressionHandler.h"
#include "ListInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	ListExpressionHandler::ListExpressionHandler()
	{
	}

	/*virtual*/ ListExpressionHandler::~ListExpressionHandler()
	{
	}

	/*virtual*/ bool ListExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		// The list may be empty, in which case, we don't find the node.
		const ParseParty::Parser::SyntaxNode* elementListNode = syntaxNode->FindChild("list-element-list", 1);
		if (elementListNode)
		{
			// We assume here that the list in question is already on the eval-stack.
			for (const ParseParty::Parser::SyntaxNode* elementNode : *elementListNode->childList)
			{
				// Push the element onto the eval stack.
				if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, elementNode, error))
				{
					error.Add(std::string(elementNode->fileLocation) + "Failed to generate instructions for list-element push.");
					return false;
				}

				// Now add the element to the list.  The element gets removed from the stack, but the list should remain.
				ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>(syntaxNode->fileLocation);
				AssemblyData::Entry entry;
				entry.code = ListInstruction::PUSH_RIGHT;
				listInstruction->assemblyData->configMap.Insert("action", entry);
				instructionList.AddTail(listInstruction);
			}
		}

		return true;
	}
}