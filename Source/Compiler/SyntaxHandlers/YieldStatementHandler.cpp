#include "YieldStatementHandler.h"
#include "YieldInstruction.h"
#include "Assembler.h"

namespace Powder
{
	YieldStatementHandler::YieldStatementHandler()
	{
	}

	/*virtual*/ YieldStatementHandler::~YieldStatementHandler()
	{
	}

	/*virtual*/ void YieldStatementHandler::HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		YieldInstruction* yeildInstruction = Instruction::CreateForAssembly<YieldInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(yeildInstruction);
	}
}