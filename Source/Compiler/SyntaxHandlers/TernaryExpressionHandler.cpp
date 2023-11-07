#include "TernaryExpressionHandler.h"
#include "Error.h"

using namespace Powder;

TernaryExpressionHandler::TernaryExpressionHandler()
{
}

/*virtual*/ TernaryExpressionHandler::~TernaryExpressionHandler()
{
}

/*virtual*/ bool TernaryExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
{
	error.Add(std::string(syntaxNode->fileLocation) + "Not yet implemented.");
	return false;
}