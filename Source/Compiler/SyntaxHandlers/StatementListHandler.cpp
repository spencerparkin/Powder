#include "StatementListHandler.h"

namespace Powder
{
	StatementListHandler::StatementListHandler()
	{
	}

	/*virtual*/ StatementListHandler::~StatementListHandler()
	{
	}

	/*virtual*/ void StatementListHandler::HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		for (const LinkedList<Parser::SyntaxNode*>::Node* node = syntaxNode->childList.GetHead(); node; node = node->GetNext())
		{
			// We simply execute the code for each statement in order.
			instructionGenerator->GenerateInstructionListRecursively(instructionList, node->value);
		}
	}
}