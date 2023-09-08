#include "StatementListHandler.h"
#include "Error.h"

namespace Powder
{
	StatementListHandler::StatementListHandler()
	{
	}

	/*virtual*/ StatementListHandler::~StatementListHandler()
	{
	}

	/*virtual*/ bool StatementListHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		for (const ParseParty::Parser::SyntaxNode* childNode : *syntaxNode->childList)
		{
			// We simply execute the code for each statement in order.
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, childNode, error))
			{
				error.Add(std::string(childNode->fileLocation) + "Failed to generate instructions for statement.");
				return false;
			}
		}

		return true;
	}
}