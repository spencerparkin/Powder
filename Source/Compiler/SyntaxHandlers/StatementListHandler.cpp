#include "StatementListHandler.h"

namespace Powder
{
	StatementListHandler::StatementListHandler()
	{
	}

	/*virtual*/ StatementListHandler::~StatementListHandler()
	{
	}

	/*virtual*/ void StatementListHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		for (const ParseParty::Parser::SyntaxNode* childNode : *syntaxNode->childList)
		{
			// We simply execute the code for each statement in order.
			instructionGenerator->GenerateInstructionListRecursively(instructionList, childNode);
		}
	}
}