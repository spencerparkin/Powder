#include "YieldStatementHandler.h"
#include "YieldInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	YieldStatementHandler::YieldStatementHandler()
	{
	}

	/*virtual*/ YieldStatementHandler::~YieldStatementHandler()
	{
	}

	/*virtual*/ bool YieldStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		YieldInstruction* yeildInstruction = Instruction::CreateForAssembly<YieldInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(yeildInstruction);
		return true;
	}
}