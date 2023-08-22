#include "ListPushPopExpressionHandler.h"
#include "ListInstruction.h"
#include "StoreInstruction.h"
#include "PopInstruction.h"
#include "Assembler.h"

namespace Powder
{
	ListPushPopExpressionHandler::ListPushPopExpressionHandler()
	{
	}

	/*virtual*/ ListPushPopExpressionHandler::~ListPushPopExpressionHandler()
	{
	}

	/*virtual*/ void ListPushPopExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 3)
			throw new CompileTimeException("Expected \"list-push-pop-expression\" in AST to have exactly 3 children.", &syntaxNode->fileLocation);

		const ParseParty::Parser::SyntaxNode* actionNode = syntaxNode->childList.GetHead()->GetNext()->value;
		if (*actionNode->name == "-->" || *actionNode->name == "<--")
		{
			// Push the list onto the eval stack.  Note that if it's not a list, we'll only know at run-time.
			const ParseParty::Parser::SyntaxNode* listNode = (*actionNode->name == "-->") ? syntaxNode->childList.GetHead()->value : syntaxNode->childList.GetHead()->GetNext()->GetNext()->value;
			LinkedList<Instruction*> listInstructionList;
			instructionGenerator->GenerateInstructionListRecursively(listInstructionList, listNode);
			instructionList.Append(listInstructionList);

			// Now issue the list instruction to pop left or right.  The list is replaced with the popped value on the eval stack.
			ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>(syntaxNode->fileLocation);
			AssemblyData::Entry entry;
			entry.code = (*actionNode->name == "-->") ? ListInstruction::POP_RIGHT : ListInstruction::POP_LEFT;
			listInstruction->assemblyData->configMap.Insert("action", entry);
			instructionList.AddTail(listInstruction);

			// TODO: Support "list --> other_container[key]"?

			// Lastly, store the popped value into the given identifier.
			const ParseParty::Parser::SyntaxNode* identifierNode = (*actionNode->name == "-->") ? syntaxNode->childList.GetHead()->GetNext()->GetNext()->value : syntaxNode->childList.GetHead()->value;
			if (*identifierNode->name != "identifier")
				throw new CompileTimeException(FormatString("List pop expected to store value in location given by name, but got no identifier.  Got \"%s\" instead.", identifierNode->name->c_str()), &syntaxNode->fileLocation);
			StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(syntaxNode->fileLocation);
			entry.Reset();
			entry.string = *identifierNode->childList.GetHead()->value->name;
			storeInstruction->assemblyData->configMap.Insert("name", entry);
			instructionList.AddTail(storeInstruction);
		}
		else if (*actionNode->name == "--<" || *actionNode->name == ">--")
		{
			// Push the list onto the eval stack.  Note that if it's not a list, we'll only know at run-time.
			const ParseParty::Parser::SyntaxNode* listNode = (*actionNode->name == "--<") ? syntaxNode->childList.GetHead()->value : syntaxNode->childList.GetHead()->GetNext()->GetNext()->value;
			LinkedList<Instruction*> listInstructionList;
			instructionGenerator->GenerateInstructionListRecursively(listInstructionList, listNode);
			instructionList.Append(listInstructionList);

			// Now push the element onto the eval stack.  This can be anything, even another list or map.
			const ParseParty::Parser::SyntaxNode* elementNode = (*actionNode->name == "--<") ? syntaxNode->childList.GetHead()->GetNext()->GetNext()->value : syntaxNode->childList.GetHead()->value;
			LinkedList<Instruction*> elementInstructionList;
			instructionGenerator->GenerateInstructionListRecursively(elementInstructionList, elementNode);
			instructionList.Append(elementInstructionList);

			// Now issue the push instruction.  The element value will be consumed, and the list will remain on the eval stack.
			ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>(syntaxNode->fileLocation);
			AssemblyData::Entry entry;
			entry.code = (*actionNode->name == "--<") ? ListInstruction::PUSH_RIGHT : ListInstruction::PUSH_LEFT;
			listInstruction->assemblyData->configMap.Insert("action", entry);
			instructionList.AddTail(listInstruction);

			// Now check our context.  If we're an immediate child of a statement-list, then no one wants the list anymore.  Don't leak the list on the eval-stack.
			if (this->PopNeededForExpression(syntaxNode))
			{
				PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
				instructionList.AddTail(popInstruction);
			}
		}
		else
		{
			throw new CompileTimeException(FormatString("Did not recognize action (%s) for list manipulation operation.", actionNode->name->c_str()), &actionNode->fileLocation);
		}
	}
}