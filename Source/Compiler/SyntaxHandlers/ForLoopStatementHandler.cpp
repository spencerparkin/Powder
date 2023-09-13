#include "ForLoopStatementHandler.h"
#include "Error.h"

using namespace Powder;

ForLoopStatementHandler::ForLoopStatementHandler()
{
}

/*virtual*/ ForLoopStatementHandler::~ForLoopStatementHandler()
{
}

/*virtual*/ bool ForLoopStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
{
	error.Add(std::string(syntaxNode->fileLocation) + "For-loop not yet implemented.");
	return false;
}