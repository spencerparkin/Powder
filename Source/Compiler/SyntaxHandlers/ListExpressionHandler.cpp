#include "ListExpressionHandler.h"
#include "ListInstruction.h"
#include "Assembler.h"

namespace Powder
{
	ListExpressionHandler::ListExpressionHandler()
	{
	}

	/*virtual*/ ListExpressionHandler::~ListExpressionHandler()
	{
	}

	/*virtual*/ void ListExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		// The list may be empty, in which case, we don't find the node.
		const ParseParty::Parser::SyntaxNode* elementListNode = syntaxNode->FindChild("list-element-list", 1);
		if (elementListNode)
		{
			// We assume here that the list in question is already on the eval-stack.
			for (const LinkedList<ParseParty::Parser::SyntaxNode*>::Node* node = elementListNode->childList.GetHead(); node; node = node->GetNext())
			{
				// Push the element onto the eval stack.
				instructionGenerator->GenerateInstructionListRecursively(instructionList, node->value);

				// Now add the element to the list.  The element gets removed from the stack, but the list should remain.
				ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>(syntaxNode->fileLocation);
				AssemblyData::Entry entry;
				entry.code = ListInstruction::PUSH_RIGHT;
				listInstruction->assemblyData->configMap.Insert("action", entry);
				instructionList.AddTail(listInstruction);
			}
		}
	}
}